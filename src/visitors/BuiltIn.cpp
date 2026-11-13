#include "headers/BuiltIn.h"
#include "headers/TomlType.h"
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
                parseTomlType(argsArr[i]),
                "arg" + std::to_string(i + 1)
            );
        }

        // Функции с pure = true можно вызывать из @pure
        std::vector<std::string> labels;
        if (toml::find_or<bool>(func, "pure", false))
            labels.push_back("@pure");

        registry.addBuiltinFunction(
            name,
            std::make_shared<FunctionNode>(
                name,
                "",
                parseTomlType(retStr),
                args,
                labels,
                nullptr
            )
        );
        #if DEBUG
            std::cout << "Добавлена встроенная функция: " << name << std::endl;
        #endif
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
    registry.addBuiltinType("f64", std::make_shared<SimpleTypeNode>("f64"));
    registry.addBuiltinType("char", std::make_shared<SimpleTypeNode>("char"));
    registry.addBuiltinType("func", std::make_shared<SimpleTypeNode>("func")); // func<параметры..., результат>
    registry.addBuiltinType("panic_info", std::make_shared<SimpleTypeNode>("panic_info"));
    registry.addBuiltinType("result", std::make_shared<SimpleTypeNode>("result")); // result<T>: значение или ошибка
    registry.addBuiltinType("string", std::make_shared<SimpleTypeNode>("string"));
    registry.addBuiltinType("null", std::make_shared<SimpleTypeNode>("null"));
    registry.addBuiltinType("none", std::make_shared<SimpleTypeNode>("none"));
    registry.addBuiltinType("auto", std::make_shared<SimpleTypeNode>("auto"));

    // Добавление встроенных типов массивов и карт
    // Потому что один хуй они являются base типами для GenericTypeNode
    registry.addBuiltinType("array", std::make_shared<SimpleTypeNode>("array"));
    registry.addBuiltinType("map", std::make_shared<SimpleTypeNode>("map"));
}