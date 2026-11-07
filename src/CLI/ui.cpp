#include "headers/ui.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace ui {

const char* const RED = "\033[31m";
const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[33m";
const char* const CYAN = "\033[36m";
const char* const DIM = "\033[2m";
const char* const BOLD = "\033[1m";
const char* const RESET = "\033[0m";

bool isTerminal(FILE* stream) {
#if defined(_WIN32) || defined(_WIN64)
    return false;
#else
    return isatty(fileno(stream));
#endif
}

bool colorEnabled(FILE* stream) {
    if (std::getenv("NO_COLOR")) return false;
    const char* term = std::getenv("TERM");
    if (term && std::strcmp(term, "dumb") == 0) return false;
    return isTerminal(stream);
}

std::string style(FILE* stream, const char* code) {
    return colorEnabled(stream) ? code : "";
}

static int terminalWidth(FILE* stream) {
#if !defined(_WIN32) && !defined(_WIN64)
    winsize size{};
    if (ioctl(fileno(stream), TIOCGWINSZ, &size) == 0 && size.ws_col > 0) return size.ws_col;
#endif
    return 80;
}

static const std::vector<std::string> BANNER = {
    "███╗   ███╗ ██████╗ ███╗   ██╗ ██████╗ ███████╗ ██████╗██████╗ ██╗██████╗ ████████╗",
    "████╗ ████║██╔═══██╗████╗  ██║██╔═══██╗██╔════╝██╔════╝██╔══██╗██║██╔══██╗╚══██╔══╝",
    "██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║███████╗██║     ██████╔╝██║██████╔╝   ██║   ",
    "██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║╚════██║██║     ██╔══██╗██║██╔═══╝    ██║   ",
    "██║ ╚═╝ ██║╚██████╔╝██║ ╚████║╚██████╔╝███████║╚██████╗██║  ██║██║██║        ██║   ",
    "╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝╚═╝        ╚═╝   ",
};
static constexpr int BANNER_WIDTH = 84;

static size_t utf8Length(unsigned char first) {
    if (first < 0x80) return 1;
    if (first < 0xE0) return 2;
    if (first < 0xF0) return 3;
    return 4;
}

// Перелив от голубого к пурпурному по ширине арта
static void printGradientLine(FILE* stream, const std::string& line) {
    static const int palette[] = {51, 45, 39, 33, 63, 99, 135, 171, 207, 213};
    constexpr int colors = sizeof(palette) / sizeof(palette[0]);

    int column = 0;
    int lastColor = -1;
    for (size_t i = 0; i < line.size();) {
        const size_t length = utf8Length(static_cast<unsigned char>(line[i]));
        const int color = palette[column * colors / BANNER_WIDTH < colors ? column * colors / BANNER_WIDTH : colors - 1];
        if (color != lastColor) {
            std::fprintf(stream, "\033[38;5;%dm", color);
            lastColor = color;
        }
        std::fwrite(line.data() + i, 1, length, stream);
        i += length;
        column++;
    }
    std::fputs(RESET, stream);
}

void printBanner(FILE* stream) {
    if (terminalWidth(stream) < BANNER_WIDTH) {
        std::fprintf(stream, "%sMONOSCRIPT%s\n", style(stream, BOLD).c_str(), style(stream, RESET).c_str());
        return;
    }

    const bool animate = isTerminal(stream);
    const bool color = colorEnabled(stream);
    for (const auto& line : BANNER) {
        if (color) printGradientLine(stream, line);
        else std::fputs(line.c_str(), stream);
        std::fputc('\n', stream);
        if (animate) {
            std::fflush(stream);
            std::this_thread::sleep_for(std::chrono::milliseconds(35));
        }
    }
}

static const char* const FRAMES[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
static constexpr int FRAME_COUNT = sizeof(FRAMES) / sizeof(FRAMES[0]);

Spinner::Spinner(std::string label_, bool keep_)
    : label(std::move(label_)), keep(keep_), active(isTerminal(stderr)), started(std::chrono::steady_clock::now()) {
    if (!active) return;

    running = true;
    worker = std::thread([this]() {
        const std::string color = style(stderr, CYAN), reset = style(stderr, RESET);
        std::unique_lock<std::mutex> lock(mutex);

        // Шаг быстрее этого мигать не должен. Ожидание прерывается сразу: анимация не должна замедлять работу
        stopped.wait_for(lock, std::chrono::milliseconds(100), [this]() { return !running; });
        for (int frame = 0; running; frame++) {
            std::fprintf(stderr, "\r\033[K%s%s%s %s", color.c_str(), FRAMES[frame % FRAME_COUNT], reset.c_str(), label.c_str());
            std::fflush(stderr);
            stopped.wait_for(lock, std::chrono::milliseconds(80), [this]() { return !running; });
        }
    });
}

void Spinner::stop() {
    if (worker.joinable()) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            running = false;
        }
        stopped.notify_all();
        worker.join();
    }
}

void Spinner::finish(const char* mark, const char* color, const std::string& detail) {
    if (!active) return;
    active = false;
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count();
    stop();

    std::fputs("\r\033[K", stderr);
    if (keep) {
        const std::string note = detail.empty() ? std::to_string(elapsed) + " мс" : detail;
        std::fprintf(stderr, "%s%s%s %s %s%s%s\n", style(stderr, color).c_str(), mark, style(stderr, RESET).c_str(), label.c_str(),
                     style(stderr, DIM).c_str(), note.c_str(), style(stderr, RESET).c_str());
    }
    std::fflush(stderr);
}

void Spinner::done(const std::string& detail) { finish("✓", GREEN, detail); }
void Spinner::fail() { finish("✗", RED, "ошибка"); }

// Сюда попадаем и при исключении: строка спиннера не должна остаться перед сообщением об ошибке
Spinner::~Spinner() {
    keep = false;
    finish("", "", "");
}

static bool progressShown = false;

void showProgress(int done, int total, int failed, const std::string& name) {
    if (!isTerminal(stderr)) return;

    constexpr int barWidth = 24;
    const int filled = total > 0 ? done * barWidth / total : 0;

    std::string bar;
    for (int i = 0; i < barWidth; i++) bar += i < filled ? "█" : "░";

    const std::string color = style(stderr, failed > 0 ? RED : CYAN), reset = style(stderr, RESET), dim = style(stderr, DIM);
    const int room = terminalWidth(stderr) - barWidth - 22;
    const std::string shown = room > 0 ? name.substr(0, room) : "";

    std::fprintf(stderr, "\r\033[K%s%s %s%s %d/%d %s%s%s", color.c_str(), FRAMES[done % FRAME_COUNT], bar.c_str(), reset.c_str(),
                 done, total, dim.c_str(), shown.c_str(), reset.c_str());
    std::fflush(stderr);
    progressShown = true;
}

void clearProgress() {
    if (!progressShown) return;
    std::fputs("\r\033[K", stderr);
    std::fflush(stderr);
    progressShown = false;
}

}
