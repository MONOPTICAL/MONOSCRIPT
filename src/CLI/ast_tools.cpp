#include "headers/ast_tools.h"
#include "../lexer/headers/Lexer.h"
#include "../parser/headers/Parser.h"
#include "../linker/headers/Linker.h"
#include "../includes/ASTDebugger.hpp"
#include "../errors/headers/ErrorEngine.h"
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
    
#if DEBUG

    std::cout << "--- Модули ---" << std::endl;
#endif     
    // Выводим информацию о связанных модулях
    for (const auto& [name, module] : linker.getModules()) {
#if DEBUG
        std::cout << "Модуль: " << name << " (" << module.path << ")" << std::endl;
        std::cout << "  Функции: " << module.functions.size() << std::endl;
        std::cout << "  Глобальные переменные: " << module.globals.size() << std::endl;
        std::cout << "  Импортировано: " << module.imports.size() << " символов" << std::endl;
#endif
        // Копируем все узлы из текущего модуля в объединенный AST
        if (module.ast) {
            for (const auto& node : module.ast->body) {
                // Пропускаем директивы импорта в объединенном AST, они уже обработаны
                if (!std::dynamic_pointer_cast<ImportNode>(node)) {
                    // Добавляем метку модуля в объединенный AST
                    auto newModuleMark = std::make_shared<ModuleMark>(module.path);
                    combinedAST->body.push_back(newModuleMark);
                    
                    combinedAST->body.push_back(node);
                }
            }
        }
    }

#if DEBUG
    std::cout << "--- Конец модулей ---\n";
#endif    

    if (showAST && combinedAST) {
        std::cout << "\n--- Объединенный AST ---\n";
        ASTDebugger::debug(combinedAST);
        std::cout << "--- Конец объединенного AST ---\n";
    }

    
    // Инициализация ErrorEngine
    std::vector<std::string>* sourceFromTokens = new std::vector<std::string>();
    for (const auto& tokenList : tokens) {
        std::string sourceLine;
        for (const auto& token : tokenList) {
            sourceLine += token.value + " ";
        }
        sourceFromTokens->push_back(sourceLine);
    }

#if DEBUG
    // Выводим для дебага
    std::cout << "\n--- Исходный код из токенов ---\n";
    int lineNumber = 0;
    for (const auto& line : *sourceFromTokens) {
        std::cout << "#" << lineNumber << " " <<  line << "\n";
        lineNumber++;
    }
    std::cout << "--- Конец исходного кода из токенов ---\n";
#endif

    ErrorEngine::getInstance().initialize(*sourceFromTokens);

    return combinedAST;
}