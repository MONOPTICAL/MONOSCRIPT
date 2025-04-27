#ifndef REGISTER_H
#define REGISTER_H

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

// Вперёд-объявления
struct TypeNode;
struct FunctionNode;
struct StructNode;
struct ClassNode;

class Registry {
public:
    // Базовые типы
    std::unordered_map<std::string, std::shared_ptr<TypeNode>> builtinTypes;

    // Базовые функции
    std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionNode>>> builtinFunctions;

    // Пользовательские структуры
    std::unordered_map<std::string, std::shared_ptr<StructNode>> userStructs;

    // Пользовательские классы
    std::unordered_map<std::string, std::shared_ptr<ClassNode>> userClasses;

    // Добавление
    void addBuiltinType(const std::string& name, std::shared_ptr<TypeNode> type);
    void addBuiltinFunction(const std::string& name, std::shared_ptr<FunctionNode> func);
    void addStruct(const std::string& name, std::shared_ptr<StructNode> strct);
    void addClass(const std::string& name, std::shared_ptr<ClassNode> cls);

    // Поиск
    std::shared_ptr<TypeNode> findType(const std::string& name) const;
    std::shared_ptr<FunctionNode> findFunction(const std::string& name) const;
    std::shared_ptr<FunctionNode> findFunction(const std::string& name, const std::vector<std::shared_ptr<TypeNode>>& args) const;
    std::shared_ptr<StructNode> findStruct(const std::string& name) const;
    std::shared_ptr<ClassNode> findClass(const std::string& name) const;
};

#endif // REGISTER_H