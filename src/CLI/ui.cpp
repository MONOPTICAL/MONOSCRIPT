#include "headers/ui.h"
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace ui {

const char* const RED = "\033[31m";
const char* const ACCENT = "\033[38;5;141m";
const char* const WHITE = "\033[97m";
const char* const DIM = "\033[2m";
const char* const BOLD = "\033[1m";
const char* const RESET = "\033[0m";

// Белый на настоящем чёрном: цвет 16 не зависит от темы терминала
static const char* const BANNER_COLORS = "\033[97;48;5;16m";

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

// --- Баннер ---

static const std::vector<std::string> BANNER = {
    "███╗   ███╗ ██████╗ ███╗   ██╗ ██████╗ ███████╗ ██████╗██████╗ ██╗██████╗ ████████╗",
    "████╗ ████║██╔═══██╗████╗  ██║██╔═══██╗██╔════╝██╔════╝██╔══██╗██║██╔══██╗╚══██╔══╝",
    "██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║███████╗██║     ██████╔╝██║██████╔╝   ██║   ",
    "██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║╚════██║██║     ██╔══██╗██║██╔═══╝    ██║   ",
    "██║ ╚═╝ ██║╚██████╔╝██║ ╚████║╚██████╔╝███████║╚██████╗██║  ██║██║██║        ██║   ",
    "╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝╚═╝        ╚═╝   ",
};
static constexpr int BANNER_WIDTH = 88; // Вместе с полями чёрного фона

// Одна-две строки, каждая подсказка должна оставаться правдой: при изменении языка их надо поправить
static const std::vector<std::vector<std::string>> TIPS = {
    {"x ^= 5 declares a variable and infers its type from the value."},
    {"@strict turns off implicit casts for one function.", "Literals that fit the type are still allowed."},
    {"@pure is checked by the compiler: no impure calls,", "and no changes to the objects passed in."},
    {"Arrays, maps and structs are references.", "Use copy(x) when you need a separate shallow copy."},
    {"final i32 x can be assigned later: exactly once,", "and on every path before it is read."},
    {"An index out of bounds stops the program with its line number and exit code 101."},
    {"has(m, k) checks a map key. Reading a missing key stops the program."},
    {"for k in m walks map keys in insertion order."},
    {"range(n) counts from 0, range(a, b) counts from a. The end is never included."},
    {"cstr(x) turns a number into a string, the same way \"x=\" + n does."},
    {"Narrowing always needs a cast: i8 small = big->i8"},
    {"f64 holds about 15 digits, float about 7. Keep timestamps and totals in f64 or i64."},
    {"Blocks start with |, so nesting stays visible instead of hiding in whitespace."},
    {"ms --compile out file.ms builds a native executable."},
    {"ms run file.ms --timings shows how long each compiler stage took."},
};

void printBanner(FILE* stream) {
    const bool color = colorEnabled(stream);
    const bool animate = isTerminal(stream);

    if (terminalWidth(stream) < BANNER_WIDTH) {
        std::fprintf(stream, "%sMONOSCRIPT%s\n", style(stream, BOLD).c_str(), style(stream, RESET).c_str());
    } else {
        const std::string blank(BANNER_WIDTH, ' ');
        auto line = [&](const std::string& text) {
            if (color) std::fprintf(stream, "%s  %s  %s\n", BANNER_COLORS, text.c_str(), RESET);
            else std::fprintf(stream, "  %s\n", text.c_str());
            if (animate) {
                std::fflush(stream);
                std::this_thread::sleep_for(std::chrono::milliseconds(35));
            }
        };
        if (color) std::fprintf(stream, "%s%s%s\n", BANNER_COLORS, blank.c_str(), RESET);
        for (const auto& text : BANNER) line(text);
        if (color) std::fprintf(stream, "%s%s%s\n", BANNER_COLORS, blank.c_str(), RESET);
    }

    std::random_device random;
    const auto& tip = TIPS[random() % TIPS.size()];
    const std::string accent = style(stream, ACCENT), white = style(stream, WHITE), reset = style(stream, RESET);
    std::fprintf(stream, "\n  %stip%s  %s%s%s\n", accent.c_str(), reset.c_str(), white.c_str(), tip[0].c_str(), reset.c_str());
    if (tip.size() > 1) std::fprintf(stream, "       %s%s%s\n", white.c_str(), tip[1].c_str(), reset.c_str());
}

// --- Полоса ---

static const char* const FRAMES[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
static constexpr int FRAME_COUNT = sizeof(FRAMES) / sizeof(FRAMES[0]);
static constexpr int BAR_WIDTH = 24;

static void drawBar(int frame, int done, int total, const char* barColor, const std::string& text) {
    const int filled = total > 0 ? done * BAR_WIDTH / total : 0;
    std::string bar;
    for (int i = 0; i < BAR_WIDTH; i++) bar += i < filled ? "█" : "░";

    // Полоса всегда в одну строку, иначе \r затрёт только её конец
    const int room = terminalWidth(stderr) - BAR_WIDTH - 12;
    std::string shown;
    int width = 0;
    for (size_t i = 0; i < text.size() && width < room; width++) {
        const unsigned char first = text[i];
        const size_t length = first < 0x80 ? 1 : first < 0xE0 ? 2 : first < 0xF0 ? 3 : 4;
        shown.append(text, i, length);
        i += length;
    }

    const std::string color = style(stderr, barColor), reset = style(stderr, RESET), white = style(stderr, WHITE);
    std::fprintf(stderr, "\r\033[K%s%s %s%s %d/%d %s%s%s", color.c_str(), FRAMES[frame % FRAME_COUNT], bar.c_str(), reset.c_str(),
                 done, total, white.c_str(), shown.c_str(), reset.c_str());
    std::fflush(stderr);
}

// --- Стадии компилятора ---

namespace {
    using Clock = std::chrono::steady_clock;

    struct FinishedStage {
        std::string name;
        long long milliseconds;
    };

    struct Pipeline {
        std::mutex mutex;
        std::condition_variable stopped;
        std::thread worker;
        bool begun = false;
        bool animated = false;
        bool keepLines = false;
        bool running = false;
        int total = 0;
        int index = 0;
        std::string name;
        std::string detail;
        Clock::time_point stageStart;
        std::vector<FinishedStage> finished;
    };
}

static Pipeline& pipeline() {
    static Pipeline instance;
    return instance;
}

// Вызывается под mutex
static void finishCurrentStage(Pipeline& state) {
    if (state.name.empty()) return;

    const long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - state.stageStart).count();
    state.finished.push_back({state.name, elapsed});

    if (state.animated && state.keepLines) {
        std::fprintf(stderr, "\r\033[K%s✓%s %s %s%lld ms%s\n", style(stderr, ACCENT).c_str(), style(stderr, RESET).c_str(), state.name.c_str(),
                     style(stderr, DIM).c_str(), elapsed, style(stderr, RESET).c_str());
    }
    state.name.clear();
    state.detail.clear();
}

void pipelineStop() {
    Pipeline& state = pipeline();
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        if (!state.begun) return;
        state.begun = false;
        state.running = false;
        finishCurrentStage(state);
    }
    state.stopped.notify_all();
    if (state.worker.joinable()) state.worker.join();

    if (state.animated) {
        std::fputs("\r\033[K", stderr);
        std::fflush(stderr);
    }
}

void pipelineBegin(int totalStages, bool keepLines) {
    Pipeline& state = pipeline();
    state.begun = true;
    state.total = totalStages;
    state.keepLines = keepLines;
    state.animated = isTerminal(stderr);
    if (!state.animated) return;

    // Выход через exit() не должен застать работающий поток
    std::atexit(pipelineStop);

    state.running = true;
    state.worker = std::thread([&state]() {
        std::unique_lock<std::mutex> lock(state.mutex);
        const auto start = Clock::now();

        // Быстрая сборка не должна мигать. Ожидание прерывается сразу: анимация не замедляет компилятор
        state.stopped.wait_until(lock, start + std::chrono::milliseconds(100), [&state]() { return !state.running; });
        for (int frame = 0; state.running; frame++) {
            const std::string text = state.detail.empty() ? state.name : state.name + " > " + state.detail;
            drawBar(frame, state.index > 0 ? state.index - 1 : 0, state.total, ACCENT, text);
            state.stopped.wait_for(lock, std::chrono::milliseconds(80), [&state]() { return !state.running; });
        }
    });
}

void stage(const std::string& name) {
    Pipeline& state = pipeline();
    std::lock_guard<std::mutex> lock(state.mutex);
    if (!state.begun) return;

    finishCurrentStage(state);
    state.name = name;
    state.index++;
    state.stageStart = Clock::now();
}

void stageDetail(const std::string& detail) {
    Pipeline& state = pipeline();
    std::lock_guard<std::mutex> lock(state.mutex);
    if (state.begun) state.detail = detail;
}

static void printTimings() {
    pipelineStop();

    const auto& finished = pipeline().finished;
    long long total = 0, slowest = 1;
    for (const auto& item : finished) {
        total += item.milliseconds;
        if (item.milliseconds > slowest) slowest = item.milliseconds;
    }

    const std::string accent = style(stderr, ACCENT), white = style(stderr, WHITE), dim = style(stderr, DIM), reset = style(stderr, RESET);
    std::fprintf(stderr, "\n%sCompiler stages%s\n", white.c_str(), reset.c_str());
    for (const auto& item : finished) {
        // Отступ считаем сами: в строке цвет и многобайтовые символы, а printf считает байты
        const int filled = static_cast<int>(item.milliseconds * 20 / slowest);
        std::string bar;
        for (int i = 0; i < filled; i++) bar += "█";
        std::fprintf(stderr, "  %6lld ms  %s%s%s%s  %s%s%s\n", item.milliseconds, accent.c_str(), bar.c_str(), reset.c_str(),
                     std::string(20 - filled, ' ').c_str(), dim.c_str(), item.name.c_str(), reset.c_str());
    }
    std::fprintf(stderr, "  %6lld ms  total\n", total);
}

void printTimingsAtExit() {
    std::atexit(printTimings);
}

// --- Полоса тестов ---

static bool progressShown = false;

void showProgress(int done, int total, int failed, const std::string& name) {
    if (!isTerminal(stderr)) return;
    drawBar(done, done, total, failed > 0 ? RED : ACCENT, name);
    progressShown = true;
}

void clearProgress() {
    if (!progressShown) return;
    std::fputs("\r\033[K", stderr);
    std::fflush(stderr);
    progressShown = false;
}

}
