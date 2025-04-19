#ifndef LINKER_H
#define LINKER_H

#include "../../lexer/headers/Lexer.h"
#include "../../includes/icecream.hpp"
#include "../../visitors/headers/TypeSymbolVisitor.h"

#include <string>
#include <vector>
#include <optional>
// Forward declarations для типов из Parser.h
class Parser;
class ProgramNode;
class FunctionNode;
class StructNode;
class ASTNode;

// Информация о функции
struct FunctionInfo {
    std::shared_ptr<FunctionNode>                                                                       node;
    std::string                                                                                         returnType;
    std::vector<std::pair<std::string, std::string>>                                                    params; // {тип, имя}
    bool                                                                                                defined = false;
};

// Информация о глобальной переменной/константе
struct VariableInfo {
    std::shared_ptr<ASTNode>                                                                            node;
    std::string                                                                                         type;
    bool                                                                                                isConst;
    bool                                                                                                defined = false;
};

// Информация о структуре
struct StructInfo {
    std::shared_ptr<StructNode>                                                                         node;
    bool                                                                                                defined = false;
};

// Информация о символе (для импорта)
struct SymbolInfo {
    enum class Type {
        FUNCTION,
        VARIABLE,
        STRUCT
    };

    Type                                                                                                type;
    std::string                                                                                         name;
    std::string                                                                                         moduleName;
    
    // Для удобства сравнения
    bool                                                                                                operator==(const SymbolInfo& other) const {
                                                                                                            return name == other.name && moduleName == other.moduleName && type == other.type;
                                                                                                        }   
};

// Контекст модуля
struct ModuleContext {
    std::string                                                                                         name;
    std::string                                                                                         path;
    std::shared_ptr<ProgramNode>                                                                        ast;
    
    // Символы, определённые в этом модуле
    std::unordered_map<std::string, FunctionInfo>                                                       functions;
    std::unordered_map<std::string, VariableInfo>                                                       globals;
    std::unordered_map<std::string, StructInfo>                                                         structs;
    
    // Импортированные символы
    std::vector<SymbolInfo>                                                                             imports;
    bool                                                                                                processed = false;
};

class Linker {
public:
                                                                                                        Linker(const std::string& stdLibPath);
    
    // Добавление модуля
    bool                                                                                                addModule(
                                                                                                            const std::string& name, 
                                                                                                            const std::string& path, 
                                                                                                            std::shared_ptr<ProgramNode> ast);
    
    // Линковка модулей
    bool                                                                                                linkModules();
    
    // Получение всех AST после линковки
    std::vector<std::shared_ptr<ProgramNode>>                                                           getLinkedASTs();
    
    // Проверка корректности импортов
    bool                                                                                                validateImports();
    
    const std::unordered_map<std::string, ModuleContext>&                                               getModules() const { 
                                                                                                            return modules; 
                                                                                                        }
private:
    std::string                                                                                         stdLibPath;
    std::unordered_map<std::string, ModuleContext>                                                      modules;
    
    // Первый проход: сбор информации о модулях
    void                                                                                                collectModuleInfo(ModuleContext& module);
    
    void                                                                                                debugModules() const;

    // Обработка импортов модуля
    bool                                                                                                processImports(ModuleContext& module);
    
    // Загрузка модуля по пути
    bool                                                                                                loadModule(const std::string& modulePath);
    
    // Вспомогательная хуйня
    bool                                                                                                symbolExists(const std::string& moduleName, 
                                                                                                            const std::string& symbolName);

    SymbolInfo::Type                                                                                    getSymbolType(const std::string& moduleName, 
                                                                                                            const std::string& symbolName);
};

#endif // LINKER_H