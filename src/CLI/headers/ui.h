#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>

// Оформление CLI. Цвет и анимация только в настоящем терминале: в конвейере и в тестах вывод остаётся простым текстом.
// Анимация пишет в stderr, поэтому вывод программы ей не мешает
namespace ui {
    bool isTerminal(FILE* stream);
    bool colorEnabled(FILE* stream);

    // Цвет для потока, или пустая строка, если цвета там нет
    std::string style(FILE* stream, const char* code);
    extern const char* const RED;
    extern const char* const GREEN;
    extern const char* const YELLOW;
    extern const char* const CYAN;
    extern const char* const DIM;
    extern const char* const BOLD;
    extern const char* const RESET;

    // ASCII-арт с цветным переливом, в терминале проявляется построчно
    void printBanner(FILE* stream);

    // Шаг работы со спиннером.
    // keep: после завершения остаётся строка с галочкой. Иначе строка стирается, и экран чистый для вывода программы
    class Spinner {
    public:
        explicit Spinner(std::string label, bool keep = true);
        ~Spinner();
        Spinner(const Spinner&) = delete;
        Spinner& operator=(const Spinner&) = delete;

        void done(const std::string& detail = "");
        void fail();

    private:
        void stop();
        void finish(const char* mark, const char* color, const std::string& detail);

        std::string label;
        bool keep;
        bool active;
        bool running = false;
        std::mutex mutex;
        std::condition_variable stopped;
        std::thread worker;
        std::chrono::steady_clock::time_point started;
    };

    // Живая полоса для прогона тестов. Перед итогами стирается
    void showProgress(int done, int total, int failed, const std::string& name);
    void clearProgress();
}
