#include "lexer/headers/Lexer.h"
#include "lexer/headers/Token.h"
#include "parser/headers/Parser.h"
#include "includes/ASTDebugger.hpp"
#include "runtime/headers/CodeGenContext.h" 
#include "runtime/headers/ASTVisitors.h"
#include <llvm/IR/Module.h>         // Для llvm::Module
#include <llvm/Support/raw_ostream.h> // Для llvm::outs()
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Функция вывода справки
void printHelp(const char* programName) {
    std::cout << "Использование: " << programName << " [ОПЦИИ] [ФАЙЛ]\n\n"
              << "Опции:\n"
              << "  --help                 Показать эту справку и выйти\n"
              << "  --run                  Запустить программу (пока не реализовано)\n"
              << "  --debug-all            Показать все этапы компиляции (токены, AST, IR)\n"
              << "  --debug-tokens         Показать только токены\n"
              << "  --debug-ast            Показать только AST\n"
              << "  --debug-ir             Показать только IR\n"
              << "\nЕсли ФАЙЛ не указан, ввод читается со стандартного ввода.\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    // Флаги для отладки
    bool showTokens = false;
    bool showAST = false;
    bool showIR = false;
    bool shouldRun = false;
    std::string inputFile = "test_module";
    
    // Парсинг аргументов командной строки
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "--debug-all") {
            showTokens = showAST = showIR = true;
        } else if (arg == "--debug-tokens") {
            showTokens = true;
        } else if (arg == "--debug-ast") {
            showAST = true;
        } else if (arg == "--debug-ir") {
            showIR = true;
        } else if (arg == "--run") {
            shouldRun = true;
        } else if (arg[0] != '-') {
            // Предполагаем, что это имя входного файла
            inputFile = arg;
        } else {
            std::cerr << "Неизвестная опция: " << arg << "\n";
            std::cerr << "Используйте --help для получения справки.\n";
            return 1;
        }
    }
    
    // Если нет флагов отладки и не указан флаг запуска, по умолчанию показываем всё
    if (!showTokens && !showAST && !showIR && !shouldRun) {
        showTokens = showAST = showIR = true;
    }
    
    // Чтение исходного кода
    std::string sourceCode;
    
    if (!inputFile.empty()) {
        // Чтение из файла
        std::ifstream file(inputFile);
        if (!file) {
            std::cerr << "Ошибка: не удалось открыть файл " << inputFile << std::endl;
            return 1;
        }
        
        // Чтение всего файла
        std::string line;
        while (std::getline(file, line)) {
            sourceCode += line + "\n";
        }
    } else {
        // Чтение со стандартного ввода
        std::cout << "Введите исходный код (завершите ввод Ctrl+Z в Windows или Ctrl+D в Linux/Mac):" << std::endl;
        std::string line;
        while (std::getline(std::cin, line)) {
            sourceCode += line + "\n";
        }
    }
    
    try {
        // Лексический анализ
        Lexer lexer(sourceCode);
        lexer.tokenize();
        const std::vector<std::vector<Token>> tokens = lexer.getTokens();
        
        // Вывод токенов, если требуется
        if (showTokens) {
            std::cout << "\n--- Токены ---" << std::endl;
            lexer.printTokens();
            std::cout << "--- Конец токенов ---\n" << std::endl;
        }
        
        // Синтаксический анализ
        Parser parser(tokens);
        std::shared_ptr<ProgramNode> program = parser.parse();
        
        if (program) {
            // Вывод AST, если требуется
            if (showAST) {
                std::cout << "\n--- AST ---" << std::endl;
                ASTDebugger::debug(program);
                std::cout << "--- Конец AST ---\n" << std::endl;
            }
            
            // Генерация и вывод IR, если требуется
            if (showIR) {
                // Создаем контекст кодогенерации
                CodeGenContext context(inputFile);
                ASTGen visitor(context);
                
                // Pass visitor as a non-temporary variable
                program->accept(visitor);
                
                // Выводим сгенерированный IR
                std::cout << "\n--- LLVM IR ---" << std::endl;
                context.TheModule->print(llvm::outs(), nullptr);
                std::cout << "--- Конец LLVM IR ---\n" << std::endl;
            }
            
            // Запуск программы, если требуется
            if (shouldRun) {
                std::cout << "\n--- Запуск программы ---" << std::endl;
                std::cout << "Запуск пока не реализован. Работа над IR продолжается." << std::endl;
                std::cout << "--- Конец запуска ---\n" << std::endl;
            }
        } else {
            std::cerr << "Ошибка: не удалось разобрать исходный код." << std::endl;
            return 1;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Ошибка выполнения: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}