#include "headers/Linker.h"
#include "../parser/headers/Parser.h" 
#include "../includes/ASTDebugger.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

Linker::Linker(const std::string& stdLibPath) : stdLibPath(stdLibPath) {}

bool Linker::addModule(const std::string& name, const std::string& path, std::shared_ptr<ProgramNode> ast) {
    if (modules.find(name) != modules.end()) {
        std::cerr << "Error: Module with name " << name << " already exists" << std::endl;
        return false;
    }
    
    ModuleContext module;
    module.name = name;
    module.path = path;
    module.ast = ast;

    std::cout << "Adding module: " << path << std::endl;
    
    modules[name] = module;
    return true;
}

void Linker::collectModuleInfo(ModuleContext& module) {
    if (module.processed) return;
    
    // Проходимся по AST модуля и собираем информацию
    for (auto& node : module.ast->body) {
        // Глобальные переменные
        if (auto varNode = std::dynamic_pointer_cast<VariableAssignNode>(node)) {
            VariableInfo info;
            info.node = varNode;
            info.type = varNode->type->toString();
            info.isConst = varNode->isConst;
            info.defined = true;
            module.globals[varNode->name] = info;
        }
        // Функции
        else if (auto funcNode = std::dynamic_pointer_cast<FunctionNode>(node)) {
            FunctionInfo info;
            info.node = funcNode;
            info.returnType = funcNode->returnType->toString();
            
            for (auto& param : funcNode->parameters) {
                info.params.push_back({param.first->toString(), param.second});
            }
            
            info.defined = true;
            module.functions[funcNode->name] = info;
        }
        // Структуры
        else if (auto structNode = std::dynamic_pointer_cast<StructNode>(node)) {
            StructInfo info;
            info.node = structNode;
            info.defined = true;
            module.structs[structNode->name] = info;
        }
    }
    
    module.processed = true;
}

bool Linker::processImports(ModuleContext& module) {
    
    // Проходимся по AST и обрабатываем импорты
    for (auto& node : module.ast->body) {
        if (auto importNode = std::dynamic_pointer_cast<ImportNode>(node)) {
            for (const auto& [path, alias] : importNode->paths) {
                if (path.empty()) continue;
                
                std::string moduleName = path[0];

                // Проверяем существует ли модуль ваще
                if (modules.find(moduleName) == modules.end()) {
                    // Пытаемся загрузить модуль, если он ещё не загружен
                    if (!loadModule(moduleName)) {
                        std::cerr << "Ошибка: Не удалось найти модуль " << moduleName << std::endl;
                        return false;
                    }
                }
                collectModuleInfo(modules[moduleName]);

                // Если путь длиннее 1, значит импортируется конкретный символ
                if (path.size() > 1) {
                    std::string symbolName = path[1];
                    
                    // Проверяем, существует ли символ в модуле
                    if (!symbolExists(moduleName, symbolName)) {
                        std::cerr << "Ошибка: Символ " << symbolName << " не найден в модуле " << moduleName << std::endl;
                        return false;
                    }
                    
                    // Добавляем импортированный символ
                    SymbolInfo symbol;
                    symbol.name = symbolName;
                    symbol.moduleName = moduleName;
                    symbol.type = getSymbolType(moduleName, symbolName);
                    
                    module.imports.push_back(symbol);
                }
                // Иначе импортируется весь модуль
                else {

                    // Добавляем все функции
                    for (const auto& [funcName, funcInfo] : modules[moduleName].functions) {
                        SymbolInfo symbol;
                        symbol.name = funcName;
                        symbol.moduleName = moduleName;
                        symbol.type = SymbolInfo::Type::FUNCTION;
                        
                        module.imports.push_back(symbol);
                    }
                    
                    // Добавляем все глобальные переменные
                    for (const auto& [varName, varInfo] : modules[moduleName].globals) {
                        SymbolInfo symbol;
                        symbol.name = varName;
                        symbol.moduleName = moduleName;
                        symbol.type = SymbolInfo::Type::VARIABLE;
                        
                        module.imports.push_back(symbol);
                    }
                    
                    // Добавляем все структуры
                    for (const auto& [structName, structInfo] : modules[moduleName].structs) {
                        SymbolInfo symbol;
                        symbol.name = structName;
                        symbol.moduleName = moduleName;
                        symbol.type = SymbolInfo::Type::STRUCT;
                        
                        module.imports.push_back(symbol);
                    }
                }
            }
        }
    }
    
    return true;
}

bool Linker::loadModule(const std::string& moduleName) {
    std::string filePath;
    std::string moduleContent;
    
    // Проверяем стандартную библиотеку
    std::string stdPath = stdLibPath + "/" + moduleName + ".ms";
    if (std::filesystem::exists(stdPath)) {
        filePath = stdPath;
    } else {
        // Проверяем относительные пути
        for (const auto& [name, module] : modules) {
            if (module.path.empty()) continue;
            
            std::string dirPath = std::filesystem::path(module.path).parent_path().string();
            std::string relativePath = dirPath + "/" + moduleName + ".ms";
            
            if (std::filesystem::exists(relativePath)) {
                filePath = relativePath;
                break;
            }
        }
    }
    
    // Если файл не найден
    if (filePath.empty()) {
        return false;
    }
    
    // Чтение файла модуля
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Ошибка: не удалось открыть файл модуля " << filePath << std::endl;
        return false;
    }
    
    // Чтение содержимого
    std::string line;
    while (std::getline(file, line)) {
        moduleContent += line + "\n";
    }
    
    try {
        // Лексический анализ
        Lexer lexer(moduleContent);
        lexer.tokenize();
        const auto tokens = lexer.getTokens();

        for (const auto& tokenVec : tokens) {
            this->Tokens.push_back(tokenVec);
        }
        
        // Парсинг в AST
        std::cout << "!!!!!!!!!!!!!!!!!1Parsing module: " << moduleName << std::endl;
        Parser parser(tokens, moduleName);
        auto moduleAST = parser.parse();
        
        if (!moduleAST) {
            std::cerr << "Error: failed to parse module " << moduleName << std::endl;
            return false;
        }
        
        return addModule(moduleName, filePath, moduleAST);
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading module " << moduleName << ": " << e.what() << std::endl;
        return false;
    }
}

bool Linker::symbolExists(const std::string& moduleName, const std::string& symbolName) {
    if (modules.find(moduleName) == modules.end()) return false;
    
    const auto& module = modules[moduleName];
    
    return module.functions.find(symbolName) != module.functions.end() ||
           module.globals.find(symbolName) != module.globals.end() ||
           module.structs.find(symbolName) != module.structs.end();
}

SymbolInfo::Type Linker::getSymbolType(const std::string& moduleName, const std::string& symbolName) {
    if (modules.find(moduleName) == modules.end()) 
        return SymbolInfo::Type::FUNCTION; // По умолчанию
    
    const auto& module = modules[moduleName];
    
    if (module.functions.find(symbolName) != module.functions.end())
        return SymbolInfo::Type::FUNCTION;
    
    if (module.globals.find(symbolName) != module.globals.end())
        return SymbolInfo::Type::VARIABLE;
    
    if (module.structs.find(symbolName) != module.structs.end())
        return SymbolInfo::Type::STRUCT;
    
    return SymbolInfo::Type::FUNCTION; // По умолчанию
}

bool Linker::validateImports() {
    for (auto& [name, module] : modules) {
        if (!processImports(module)) {
            return false;
        }
    }
    return true;
}

void Linker::debugModules() const {
    std::cout << "\n-----------------------\n";
    for (const auto& [name, module] : modules) {
        std::cout << "Модуль: " << name << " (Путь: " << module.path << ")\n";
        
        std::cout << "  Функции (" << module.functions.size() << "):\n";
        for (const auto& [funcName, info] : module.functions) {
            std::cout << "    " << info.returnType << " " << funcName << "(";
            for (size_t i = 0; i < info.params.size(); ++i) {
                std::cout << info.params[i].first << " : " << info.params[i].second;
                if (i < info.params.size() - 1) std::cout << ", ";
            }
            std::cout << ")\n";
        }
        
        std::cout << "  Глобальные переменные (" << module.globals.size() << "):\n";
        for (const auto& [varName, info] : module.globals) {
            std::cout << "    " << info.type << " " << varName << (info.isConst ? " (const)" : "") << "\n";
        }
        
        std::cout << "  Импорты (" << module.imports.size() << "):\n";
        for (const auto& import : module.imports) {
            std::cout << "    " << import.name << " из " << import.moduleName << " (";
            switch(import.type) {
                case SymbolInfo::Type::FUNCTION: std::cout << "функция"; break;
                case SymbolInfo::Type::VARIABLE: std::cout << "переменная"; break;
                case SymbolInfo::Type::STRUCT: std::cout << "структура"; break;
            }
            std::cout << ")\n";
        }
    }
    std::cout << "-----------------------\n\n";
}

bool Linker::linkModules() {
    // Собираем информацию о всех модулях
    for (auto& [name, module] : modules) {
        collectModuleInfo(module);
        processImports(module);
    }

    // Обрабатываем импорты
    if (!validateImports()) {
        return false;
    }
    
    return true;
}

std::vector<std::shared_ptr<ProgramNode>> Linker::getLinkedASTs() {
    std::vector<std::shared_ptr<ProgramNode>> result;
    for (auto& [name, module] : modules) {
        result.push_back(module.ast);
    }
    return result;
}