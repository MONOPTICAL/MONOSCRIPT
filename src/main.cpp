#include "CLI/headers/cli.h"
#include "CLI/headers/ast_tools.h"
#include "CLI/headers/runner.h"
#include "CLI/headers/symantic.h"
#include "CLI/headers/compile.h"
#include "CLI/headers/test_runner.h"
#include "CLI/headers/ui.h"
#include "errors/headers/ErrorEngine.h"
#include "loader/headers/loader.h"
#include <gc/gc.h>
#include <iostream>
#include <chrono>
#include <filesystem>

int main(int argc, char* argv[]) {
    // Сборщик мусора запускается первым, до любого выделения памяти программой
    GC_INIT();

    // Служебный режим: так -run_tests запускает сам себя на каждый тест
    if (argc > 1 && std::string(argv[1]) == "--check-file") {
        return checkMain(argc - 1, argv + 1);
    }

    try {
        // Парсинг аргументов
        CLIOptions options = parseArgs(argc, argv);

        // Тесты фронтенда
        if (options.runTests) {
            std::string self = loader::getExecutableDir() + "/" + std::filesystem::path(argv[0]).filename().string();
            return runFrontendTests({self, "--check-file"}, {self, "run"}, loader::findTestsPath(), options.verbose, options.inputFile);
        }

        // Чтение кода
        std::string sourceCode = readSourceCode(options.inputFile);
        
        // Токенизация
        auto tokens = tokenizeSource(sourceCode, options.showTokens);

        // Парсинг и линковка
        auto combinedAST = parseAndLinkModules(tokens, options.inputFile, options.showAST);

        // Семантический анализ
        combinedAST = symanticParseModule(combinedAST, options.showSymantic);
        
        // Выполнение только если указан флаг --run или run
        if (options.runJIT) {
            std::string currentFilePath = std::filesystem::current_path().string();
            int exitCode = runProgram(combinedAST, currentFilePath, options.showAST, options.offOptimization);
            return exitCode == RUN_FAILED ? 1 : exitCode;
        }

        if (options.compileExecutable) {
            // Компиляция в исполняемый файл
            std::string outputFile = options.ExecutableFile.empty() ? "output" : options.ExecutableFile;
            if (!compileToExecutable(combinedAST, outputFile)) {
                return 1;
            }
            std::cout << ui::style(stdout, ui::GREEN) << "Компиляция завершена." << ui::style(stdout, ui::RESET)
                      << " Исполняемый файл: " << ui::style(stdout, ui::BOLD) << outputFile << ui::style(stdout, ui::RESET) << std::endl;
            return 0;
        }
        
        // Если --run не указан, просто завершаем работу после анализа
        std::cout << ui::style(stdout, ui::GREEN) << "Анализ кода завершен." << ui::style(stdout, ui::RESET)
                  << " Для выполнения используйте флаг --run." << std::endl;
        
    } catch (const std::exception& e) {
        // Ошибки компиляции уже напечатаны, у них пустой текст
        if (e.what()[0] != '\0') std::cerr << ui::style(stderr, ui::RED) << e.what() << ui::style(stderr, ui::RESET) << std::endl;
        return 1;
    }
    
    return 0;
}