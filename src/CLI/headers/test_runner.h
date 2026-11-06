#pragma once
#include <string>
#include <vector>

// Проверка одного файла: 0 - код принят, 1 - ошибка компиляции, остальное - компилятор упал.
// argv[0] пропускается, дальше [--parse-only] FILE
int checkMain(int argc, char* argv[]);

// Гоняет все .ms с директивами из testsDir. checkCommand - команда, которая запускает проверку
// одного файла в отдельном процессе. runCommand запускает программу для expect-output,
// если он пустой (сборка без LLVM), такие тесты пропускаются. 0 - неожиданных результатов нет, 1 - есть.
int runFrontendTests(const std::vector<std::string>& checkCommand, const std::vector<std::string>& runCommand, const std::string& testsDir, bool verbose, const std::string& filter);
