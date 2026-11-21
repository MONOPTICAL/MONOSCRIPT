#pragma once
#include <cstdio>
#include <string>

// Оформление CLI. Цвет и анимация только в настоящем терминале: в конвейере и в тестах вывод остаётся простым текстом.
// Анимация пишет в stderr и останавливается до старта программы, поэтому её вывод ничему не мешает
namespace ui {
    bool isTerminal(FILE* stream);
    bool colorEnabled(FILE* stream);
    extern bool forceColor; // Цвет даже в трубе: REPL печатает ошибки компилятора за него

    // Цвет для потока, или пустая строка, если цвета там нет
    std::string style(FILE* stream, const char* code);
    extern const char* const RED;
    extern const char* const ACCENT; // Фиолетовый
    extern const char* const WHITE;
    extern const char* const DIM;
    extern const char* const BOLD;
    extern const char* const RESET;

    // ASCII-арт, белый на чёрном, и подсказка о языке
    void printBanner(FILE* stream);

    // Стадии компилятора: живая полоса с названием стадии и замером времени.
    // Новый слой компилятора (безопасность, математические проверки) это один вызов stage и +1 в числе стадий у pipelineBegin.
    // keepLines: завершённые стадии остаются на экране строками с галочкой
    void pipelineBegin(int totalStages, bool keepLines);
    void stage(const std::string& name);
    void stageDetail(const std::string& detail); // Что именно сейчас внутри стадии, например имя функции
    void pipelineStop();                         // Стирает полосу. Обязательно перед сообщением об ошибке и запуском программы
    void printTimingsAtExit();                   // --timings

    // Живая полоса для прогона тестов. Перед итогами стирается
    void showProgress(int done, int total, int failed, const std::string& name);
    void clearProgress();
}
