#include "headers/BuiltIn.h"

void registerBuiltInFunctions(Registry& registry) 
{
    /*
        Встроенные функции:

        echo(string|i32|i64|float|bool|null|none ...) -> void
            - Выводит значения на экран.

        len(string|array|map) -> i32
            - Возвращает длину строки, массива или карты.

        exit(i32 code) -> void
            - Завершает выполнение программы с указанным кодом.

        assert(bool condition) -> void
            - Прерывает выполнение программы, если условие ложно.
    */

    registry.addBuiltinFunction(
        "echo",                                                                             // Имя функции
        std::make_shared<FunctionNode>(
            "echo",                                                                         // Имя функции
            "",                                                                             // Ассоциированное имя
            std::make_shared<SimpleTypeNode>("void"),                                       // Тип возвращаемого значения
            std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>
            {
                std::make_pair(std::make_shared<SimpleTypeNode>("string"), "arg1"),         
                std::make_pair(std::make_shared<SimpleTypeNode>("i32"), "arg2"),
                std::make_pair(std::make_shared<SimpleTypeNode>("i64"), "arg3"),
                std::make_pair(std::make_shared<SimpleTypeNode>("float"), "arg4"),
                std::make_pair(std::make_shared<SimpleTypeNode>("bool"), "arg5"),
                std::make_pair(std::make_shared<SimpleTypeNode>("null"), "arg6"),
                std::make_pair(std::make_shared<SimpleTypeNode>("none"), "arg7")
            },                                                                              // Все возможные аргументы                                  
            nullptr                                                                         // Тело функции
        )
    );

    registry.addBuiltinFunction(
        "len",                                                                              // Имя функции
        std::make_shared<FunctionNode>(
            "len",                                                                          // Имя функции                                        
            "",                                                                             // Ассоциированное имя                                  
            std::make_shared<SimpleTypeNode>("i32"),                                        // Тип возвращаемого значения
            std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>
            {
                std::make_pair(std::make_shared<SimpleTypeNode>("string"), "arg1"),
                std::make_pair(std::make_shared<SimpleTypeNode>("array"), "arg2"),              // Даже если там параметры, то все равно baseType является SimpleTypeNode.array
                std::make_pair(std::make_shared<SimpleTypeNode>("map"), "arg3")                 // Даже если там параметры, то все равно baseType является SimpleTypeNode.map
            
            },                                                                              // Все возможные аргументы      
            nullptr                                                                         // Тело функции 
        )
    );

    registry.addBuiltinFunction(
        "exit",                                                                             // Имя функции
        std::make_shared<FunctionNode>(
            "exit", 
            "", 
            std::make_shared<SimpleTypeNode>("void"),                                       // Тип возвращаемого значения
            std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>
            {
                std::make_pair(std::make_shared<SimpleTypeNode>("i32"), "arg1")             // Код завершения

            },                                                                              // Все возможные аргументы

            nullptr                                                                         // Тело функции
        )
    );
    
    registry.addBuiltinFunction(
        "assert",                                                                           // Имя функции
        std::make_shared<FunctionNode>(
            "assert", 
            "", 
            std::make_shared<SimpleTypeNode>("void"),                                       // Тип возвращаемого значения
            std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>
            {
                std::make_pair(std::make_shared<SimpleTypeNode>("bool"), "arg1")       // Условие
            },                                                                              // Все возможные аргументы

            nullptr                                                                         // Тело функции
        )
    );

    // Cast функции
    registry.addBuiltinFunction(
        "toString",                                                                         // Имя функции
        std::make_shared<FunctionNode>(
            "toString", 
            "", 
            std::make_shared<SimpleTypeNode>("string"),                                      // Тип возвращаемого значения
            std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>
            {
                std::make_pair(std::make_shared<SimpleTypeNode>("i32"), "arg1"),
                std::make_pair(std::make_shared<SimpleTypeNode>("i64"), "arg2"),
                std::make_pair(std::make_shared<SimpleTypeNode>("float"), "arg3"),
                std::make_pair(std::make_shared<SimpleTypeNode>("bool"), "arg4"),
                std::make_pair(std::make_shared<SimpleTypeNode>("null"), "arg5"),
                std::make_pair(std::make_shared<SimpleTypeNode>("none"), "arg6")
            },                                                                              // Все возможные аргументы

            nullptr                                                                         // Тело функции
        )
    );

    registry.addBuiltinFunction(
        "toInt",                                                                           // Имя функции
        std::make_shared<FunctionNode>(
            "toInt", 
            "", 
            std::make_shared<SimpleTypeNode>("i32"),                                        // Тип возвращаемого значения
            std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>
            {
                std::make_pair(std::make_shared<SimpleTypeNode>("string"), "arg1"),
                std::make_pair(std::make_shared<SimpleTypeNode>("float"), "arg2")
            },                                                                              // Все возможные аргументы

            nullptr                                                                         // Тело функции
        )
    );

    registry.addBuiltinFunction(
        "toFloat",                                                                          // Имя функции
        std::make_shared<FunctionNode>(
            "toFloat", 
            "", 
            std::make_shared<SimpleTypeNode>("float"),                                       // Тип возвращаемого значения
            std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>
            {
                std::make_pair(std::make_shared<SimpleTypeNode>("string"), "arg1"),
                std::make_pair(std::make_shared<SimpleTypeNode>("i32"), "arg2"),
                std::make_pair(std::make_shared<SimpleTypeNode>("i64"), "arg3")
            },                                                                              // Все возможные аргументы

            nullptr                                                                         // Тело функции
        )
    );

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