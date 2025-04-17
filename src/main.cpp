#include "lexer/headers/Lexer.h"
#include "lexer/headers/Token.h"
#include "parser/headers/Parser.h"
#include "includes/ASTDebugger.hpp"
#include "visitors/headers/TypeSymbolVisitor.h"
#include <fstream>
#include <iostream>
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
            std::cout << "--- Конец токенов ---\n";
        }

        Parser parser(tokens);
        auto program = parser.parse();

        TypeSymbolVisitor typeSymbolVisitor;

        program->accept(typeSymbolVisitor);

        if (showAST && program) {
            std::cout << "\n--- AST ---\n";
            ASTDebugger::debug(program);
            std::cout << "--- Конец AST ---\n";
        } else if (!program) {
            std::cerr << "Ошибка: не удалось разобрать исходный код.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}