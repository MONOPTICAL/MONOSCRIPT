#include "lexer/headers/Lexer.h"
#include "lexer/headers/Token.h"
#include "parser/headers/Parser.h"
#include "linker/headers/Linker.h"
#include "visitors/headers/TypeSymbolVisitor.h"
#include "includes/ASTDebugger.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

void printHelp(const char* programName) {
    std::cout << "Использование: " << programName << " [ОПЦИИ] [ФАЙЛ]\n\n"
              << "Опции:\n"
              << "  --help           Показать эту справку и выйти\n"
              << "  --tokens         Показать токены\n"
              << "  --ast            Показать AST\n"
              << "\nЕсли ФАЙЛ не указан, ввод читается со стандартного ввода.\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    bool showTokens = false;
    bool showAST = false;
    std::string inputFile;

    // Парсинг аргументов
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "--tokens") {
            showTokens = true;
        } else if (arg == "--ast") {
            showAST = true;
        } else if (arg[0] != '-') {
            inputFile = arg;
        } else {
            std::cerr << "Неизвестная опция: " << arg << std::endl;
            return 1;
        }
    }
    if (!showTokens && !showAST) showTokens = showAST = true;

    // Чтение исходника
    std::string sourceCode;
    if (!inputFile.empty()) {
        std::ifstream file(inputFile);
        if (!file) {
            std::cerr << "Ошибка: не удалось открыть файл " << inputFile << std::endl;
            return 1;
        }
        std::string line;
        while (std::getline(file, line)) sourceCode += line + "\n";
    } else {
        std::cout << "Введите исходный код (Ctrl+D для завершения):\n";
        std::string line;
        while (std::getline(std::cin, line)) sourceCode += line + "\n";
    }

    try {
        Lexer lexer(sourceCode);
        lexer.tokenize();
        const auto tokens = lexer.getTokens();

        if (showTokens) {
            std::cout << "\n--- Токены ---\n";
            lexer.printTokens();
            std::cout << "--- Конец токенов ---\n\n";
        }

        Parser parser(tokens);
        auto program = parser.parse();

        // Получаем путь текущего файла
        Linker linker(std::filesystem::path(inputFile).parent_path().string());
        
        // Добавляем основной модуль
        if (!linker.addModule(std::filesystem::path(inputFile).stem().string(), inputFile, program)) {
            return 1;
        }
        
        if (!linker.linkModules()) {
            return 1;
        }
        
        auto linkedModules = linker.getLinkedASTs();
        
        // Создаем объединенный AST из всех модулей
        std::shared_ptr<ProgramNode> combinedAST = std::make_shared<ProgramNode>();

        std::cout << "--- Модули ---" << std::endl;

        // Выводим информацию о связанных модулях
        for (const auto& [name, module] : linker.getModules()) {
            std::cout << "Модуль: " << name << " (" << module.path << ")" << std::endl;
            std::cout << "  Функции: " << module.functions.size() << std::endl;
            std::cout << "  Глобальные переменные: " << module.globals.size() << std::endl;
            std::cout << "  Импортировано: " << module.imports.size() << " символов" << std::endl;
            
            // Копируем все узлы из текущего модуля в объединенный AST
            if (module.ast) {
                for (const auto& node : module.ast->body) {
                    // Пропускаем директивы импорта в объединенном AST, они уже обработаны
                    if (!std::dynamic_pointer_cast<ImportNode>(node)) {
                        combinedAST->body.push_back(node);
                    }
                }
            }
        }

        std::cout << "--- Конец модулей ---\n";

        if (showAST && combinedAST) {
            std::cout << "\n--- Объединенный AST ---\n";
            ASTDebugger::debug(combinedAST);
            std::cout << "--- Конец объединенного AST ---\n";
        }
        else if (!combinedAST) {
            std::cerr << "Ошибка: не удалось разобрать исходный код.\n";
            return 1;
        }


        TypeSymbolVisitor typeSymbolVisitor;

        combinedAST->accept(typeSymbolVisitor);

        if (showAST && combinedAST) {
            std::cout << "\n--- AST(2) ---\n";
            ASTDebugger::debug(combinedAST);
            std::cout << "--- Конец AST(2) ---\n";
        } else if (!combinedAST) {
            std::cerr << "Ошибка: не удалось разобрать исходный код.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}