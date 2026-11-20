#pragma once
#include <string>
#include <vector>

// Строка ввода для REPL: подсветка, история, стрелки, Tab-дополнение. Только для настоящего терминала
namespace lineedit {
    enum class Result { Line, Cancel, Eof }; // Cancel: Ctrl+C, Eof: Ctrl+D на пустой строке

    // prefill: с чего начинается строка, например отступ блока "|   "
    // menu: команды в строке под вводом, ←/→ на пустой строке или строке с ':' выбирают, Enter запускает
    Result read(const std::string& prompt, std::string& line, const std::string& prefill = "",
        const std::vector<std::string>& menu = {});
    void cooked();                              // Вернуть терминал программе: эхо, Ctrl+C, обычный ввод
    void addHistory(const std::string& line);
    void loadHistory(const std::string& path); // Новые строки дописываются в этот же файл

    std::string highlight(const std::string& code);
    extern std::vector<std::string> words;      // Имена сессии для Tab, сверх ключевых слов и встроенных функций
}
