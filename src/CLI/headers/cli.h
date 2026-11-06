#pragma once
#include <string>

struct CLIOptions {
    bool showTokens = false;
    bool showAST = false;
    bool showSymantic = false;
    bool runJIT = false; // LLVM JIT
    bool offOptimization = false; // LLVM Optimization
    bool compileExecutable = false; // LLVM Compile
    bool runTests = false; // Тесты фронтенда, inputFile тогда работает как фильтр по имени
    bool verbose = false; // Подробный вывод тестов
    std::string ExecutableFile;
    std::string inputFile;
};

void printHelp(const char* programName);
CLIOptions parseArgs(int argc, char* argv[]);
std::string readSourceCode(const std::string& inputFile);