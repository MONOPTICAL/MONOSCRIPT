#include "headers/BuiltIn.h"
#include "../includes/toml.hpp" // путь к toml11
#include <iostream>
void registerBuiltInFunctions(Registry& registry, const std::string& tomlPath) 
{
    const auto data = toml::parse(tomlPath);
    if (!data.contains("function")) return;

    const auto& functions = toml::find(data, "function").as_array();
    for (const auto& func : functions) {
        const auto& name = toml::find<std::string>(func, "name");
        const auto& retStr = toml::find<std::string>(func, "ret");
        const auto& argsArr = toml::find<std::vector<std::string>>(func, "args");

        // Создаём список аргументов с именами arg1, arg2, ...
        std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> args;
        for (size_t i = 0; i < argsArr.size(); ++i) {
            args.emplace_back(
                std::make_shared<SimpleTypeNode>(argsArr[i]),
                "arg" + std::to_string(i + 1)
            );
        }

        registry.addBuiltinFunction(
            name,
            std::make_shared<FunctionNode>(
                name,
                "",
                std::make_shared<SimpleTypeNode>(retStr),
                args,
                nullptr
            )
        );
        std::cout << "Добавлена встроенная функция: " << name << std::endl;
    }
}

void registerBuiltInTypes(Registry& registry) 
{
    /*
        Встроенные типы:

        i1
        i8
        i32
        i64
        float
        string
        null
        none

        array<T>
        map<K, V>
    */

    // Добавление встроенных типов
    registry.addBuiltinType("i1", std::make_shared<SimpleTypeNode>("i1"));
    registry.addBuiltinType("i8", std::make_shared<SimpleTypeNode>("i8"));
    registry.addBuiltinType("i16", std::make_shared<SimpleTypeNode>("i16"));
    registry.addBuiltinType("i32", std::make_shared<SimpleTypeNode>("i32"));
    registry.addBuiltinType("i64", std::make_shared<SimpleTypeNode>("i64"));
    registry.addBuiltinType("float", std::make_shared<SimpleTypeNode>("float"));
    registry.addBuiltinType("string", std::make_shared<SimpleTypeNode>("string"));
    registry.addBuiltinType("null", std::make_shared<SimpleTypeNode>("null"));
    registry.addBuiltinType("none", std::make_shared<SimpleTypeNode>("none"));
    registry.addBuiltinType("auto", std::make_shared<SimpleTypeNode>("auto"));

    // Добавление встроенных типов массивов и карт
    // Потому что один хуй они являются base типами для GenericTypeNode
    registry.addBuiltinType("array", std::make_shared<SimpleTypeNode>("array"));
    registry.addBuiltinType("map", std::make_shared<SimpleTypeNode>("map"));
}