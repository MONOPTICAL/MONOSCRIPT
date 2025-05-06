#include "CLI/headers/cli.h"
#include "CLI/headers/ast_tools.h"
#include "CLI/headers/runner.h"
#include "CLI/headers/symantic.h"
#include "errors/headers/ErrorEngine.h"
#include <iostream>
#include <chrono>
#include <filesystem>

int main(int argc, char* argv[]) {
    try {
        // Парсинг аргументов
        CLIOptions options = parseArgs(argc, argv);
        
        // Чтение кода
        std::string sourceCode = readSourceCode(options.inputFile);
        
        // Токенизация
        auto tokens = tokenizeSource(sourceCode, options.showTokens);

        // Парсинг и линковка
        auto combinedAST = parseAndLinkModules(tokens, options.inputFile, options.showAST);

        // Семантический анализ
        combinedAST = symanticParseModule(combinedAST, options.showSymantic);
        
        // Выполнение только если указан флаг --run или run
        if (options.run) {
            std::string currentFilePath = std::filesystem::current_path().string();
            return runProgram(combinedAST, currentFilePath, options.showAST);
        }
        
        // Если --run не указан, просто завершаем работу после анализа
        std::cout << "Анализ кода завершен. Для выполнения используйте флаг --run." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}