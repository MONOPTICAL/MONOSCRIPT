#include "../headers/TypeSymbolVisitor.h"
#include <string_view>
void TypeSymbolVisitor::visit(ProgramNode &node)
{
    for (const auto& statement : node.body) {
        statement->accept(*this);
    }
}

void TypeSymbolVisitor::visit(FunctionNode &node)
{
    // Проверяем, существует ли функция в реестре(в текущем контексте или глобальном)
    if(contexts.back().functions.find(node.name) != contexts.back().functions.end()) {
        if(contexts[0].functions.find(node.name) != contexts[0].functions.end()) {
            LogError("Function already defined in global context: " + node.name);
        }
        LogError("Function already defined: " + node.name);
    }

    if(!node.associated.empty())
    {
        if(registry.findStruct(node.associated) == nullptr) 
            LogError("Struct not found: " + node.associated);

        if(contexts.size() != 1)
            LogError("Function " + node.name + " cannot be defined in a non-global context");
    }


    /*
    Пока что не проверяем, что функция является методом класса
    Так как не реализован класс
    if (node.associated != "") {
        // Проверяем, существует ли класс в реестре
        auto it = contexts.back().variables.find(node.associated);
        if (it == contexts.back().variables.end()) {
            LogError("Class not found: " + node.associated);
        }
    }
    */
    std::unordered_map<std::string, std::shared_ptr<TypeNode>> args = {};

    // Проходим по параметрам функции
    for (const auto& param : node.parameters) {
        param.first->accept(*this); // Проверяем тип параметра
        // Тут типы параметров не проверяются, так как они могут быть разными и это нормально

        if (contexts.back().variables.find(param.second) != contexts.back().variables.end()) {
            LogError("Parameter already defined: " + param.second);
        }

        // Добавляем параметр в реестр
        args[param.second] = param.first;
    }

    // Если не вывилась ошибка, добавляем функцию в реестр
    Context currentFunction = {
        .variables = contexts.back().variables, // Добавляем переменные текущего контекста
        .functions = contexts.back().functions, // Добавляем функции текущего контекста
        .currentFunctionName = node.name,       // Имя функции 
        .returnType = node.returnType,          // Тип возвращаемого значения
        .returnedValue = false                  // Возвращаемое значение
    };

    // Добавляем параметры функции в текущий контекст
    for (const auto& param : args) {
        auto varNode = std::make_shared<VariableAssignNode>(
            param.first, false, std::dynamic_pointer_cast<TypeNode>(param.second), nullptr);
        
        varNode->inferredType = varNode->type; // <-- инициализация inferredType

        // Добавляем переменную в реестр
        currentFunction.variables[param.first] = varNode;
    }

    // Если функция находится в глобальном контексте, то добавляем ее в глобальный реестр(глобальный контекст - это первый элемент в векторе)
    // Если функция не является методом класса, то добавляем ее в глобальный реестр
    if(contexts.size() == 1)
        contexts[0].functions[node.name] = node.shared_from_this();
    else
        contexts.back().functions[node.name] = node.shared_from_this();
    
    // Добавляем функцию в реестр
    contexts.push_back(currentFunction);

    // Проверяем тело функции
    node.body->accept(*this);

    if(contexts.back().returnedValue == false && contexts.back().returnType->toString() != "void") {
        LogError("Function " + node.name + " must return a value of type " + contexts.back().returnType->toString());
    }

    contexts.pop_back(); // Убираем текущий контекст
    
    node.inferredType = node.returnType; // Устанавливаем тип функции
}

void TypeSymbolVisitor::visit(BlockNode &node)
{
    for (const auto& statement : node.statements) {
        statement->accept(*this);
    }
}

void TypeSymbolVisitor::visit(VariableAssignNode &node)
{
    // Проверяем, существует ли переменная в реестре
    if (contexts.back().variables.find(node.name) != contexts.back().variables.end()) {
        LogError("Variable already defined: " + node.name);
    }

    // Проверяем тип переменной
    node.type->accept(*this);
    std::string varType = node.type->toString();
    bool isAuto = false;

    if (varType == "auto")
        isAuto = true;

    if (
        varType == "none"
        || varType == "null"
        || varType == "void"
    ) {
        LogError("Type cannot be " + varType);
    }

    if (isAuto)
    {
        if (auto block = std::dynamic_pointer_cast<BlockNode>(node.expression))
        {
            if (std::dynamic_pointer_cast<KeyValueNode>(block->statements[0]))
            {
                auto keyValue = std::dynamic_pointer_cast<KeyValueNode>(block->statements[0]);
                keyValue->key->accept(*this);
                keyValue->value->accept(*this);

                auto genericType = std::make_shared<GenericTypeNode>("map");
                genericType->typeParameters.push_back(keyValue->key->inferredType);
                genericType->typeParameters.push_back(keyValue->value->inferredType);

                node.type = genericType;
                varType = genericType->toString();
            }
            else
            {
                auto firstStatement = block->statements[0];
                firstStatement->accept(*this);

                auto genericType = std::make_shared<GenericTypeNode>("array");
                genericType->typeParameters.push_back(firstStatement->inferredType);

                node.type = genericType;
                varType = genericType->toString();
            }
        }
    }
    // Проверяем выражение является ли оно дженериком

    debugContexts();
    
    if (varType.starts_with("array")
        || varType.starts_with("map")
    ) {
        validateCollectionElements(node.type, node.expression, isAuto);
        IC("exit");
    }
    
    else
    {
        node.expression->accept(*this);
        auto expressionType = node.expression->inferredType->toString();
        if (expressionType != "none" || expressionType != "string")
        {        
            IC(expressionType, varType);
            // Проверяем тип выражения
            castNumbersInBinaryTree(node.expression, isAuto ? "auto" : expressionType);
            if (isAuto)
            {
                
            }
        }
        IC("exitted");
    }
    // Добавляем переменную в реестр
    auto varNode = std::make_shared<VariableAssignNode>(node.name, node.isConst, node.type, node.expression);
    varNode->inferredType = node.type; // Устанавливаем тип переменной

    contexts.back().variables[node.name] = varNode;
}

void TypeSymbolVisitor::visit(ReturnNode &node)
{
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("Return statement outside of function");
    }

    // Проверяем тип возвращаемого значения
    if (node.expression) {
        node.expression->accept(*this);
        IC(node.expression->inferredType->toString());
        std::string expectedType = contexts.back().returnType->toString();
        std::string actualType = node.expression->inferredType->toString();
        if (actualType != expectedType) {
            if (!(actualType == "null" && expectedType == "void"))
                LogError("Return type mismatch: expected " + contexts.back().returnType->toString() + ", got " + node.expression->inferredType->toString());
        }
    }

    contexts.back().returnedValue = true; // Устанавливаем, что функция вернула значение
}