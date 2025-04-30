#include "headers/ast_tools.h"
#include "../lexer/headers/Lexer.h"
#include "../parser/headers/Parser.h"
#include "../linker/headers/Linker.h"
#include "../includes/ASTDebugger.hpp"
#include <iostream>
#include <filesystem>

std::vector<std::vector<Token>> tokenizeSource(const std::string& sourceCode, bool showTokens) {
    Lexer lexer(sourceCode);
    lexer.tokenize();
    
    if (showTokens) {
        std::cout << "\n--- Токены ---\n";
        lexer.printTokens();
        std::cout << "--- Конец токенов ---\n\n";
    }
    
    return lexer.getTokens();
}

std::shared_ptr<ProgramNode> parseAndLinkModules(
    const std::vector<std::vector<Token>>& tokens, 
    const std::string& inputFile, 
    bool showAST
) {
    Parser parser(tokens, inputFile);
    auto program = parser.parse();
    
    // Получаем путь текущего файла
    std::string currentFilePath = std::filesystem::current_path().string();
    Linker linker(currentFilePath);
    
    // Добавляем основной модуль
    if (!linker.addModule(std::filesystem::path(inputFile).stem().string(), inputFile, program)) {
        throw std::runtime_error("Ошибка при добавлении модуля");
    }
    
    if (!linker.linkModules()) {
        throw std::runtime_error("Ошибка при линковке модулей");
    }
    
    // Создаем объединенный AST из всех модулей
    std::shared_ptr<ProgramNode> combinedAST = std::make_shared<ProgramNode>();
    combinedAST->moduleName = program->moduleName;
    
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
    
    return combinedAST;
}