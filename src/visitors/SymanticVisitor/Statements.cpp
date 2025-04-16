#include "../headers/SymanticVisitor.h"

void SymanticVisitor::visit(ProgramNode &node)
{
    for (const auto& statement : node.body) {
        statement->accept(*this);
    }
}

void SymanticVisitor::visit(FunctionNode &node)
{
    // Проверяем, существует ли функция в реестре(в текущем контексте или глобальном)
    if(contexts.back().functions.find(node.name) != contexts.back().functions.end()) {
        if(contexts[0].functions.find(node.name) != contexts[0].functions.end()) {
            LogError("Function already defined in global context: " + node.name);
        }
        LogError("Function already defined: " + node.name);
    }
    
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> args = {};

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
        currentFunction.variables[param.first] = std::make_shared<VariableAssignNode>(param.first, false, std::dynamic_pointer_cast<TypeNode>(param.second), nullptr);
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

void SymanticVisitor::visit(BlockNode &node)
{
    for (const auto& statement : node.statements) {
        statement->accept(*this);
    }
}

void SymanticVisitor::visit(VariableAssignNode &node)
{
    // Проверяем, существует ли переменная в реестре
    if (contexts.back().variables.find(node.name) != contexts.back().variables.end()) {
        LogError("Variable already defined: " + node.name);
    }

    // Проверяем тип переменной
    node.type->accept(*this);

    // Добавляем переменную в реестр
    contexts.back().variables[node.name] = std::make_shared<VariableAssignNode>(node.name, node.isConst, node.type, nullptr);
    
    if (auto varAssign =std::dynamic_pointer_cast<VariableAssignNode>(contexts.back().variables[node.name])) {
        
        IC( 
            varAssign->name ,
            varAssign->type->toString() ,
            varAssign->isConst
        );

    }

    // Проверяем выражение
    node.expression->accept(*this);
    
    // Проверяем тип выражения
    if (node.expression->inferredType->toString() != node.type->toString()) {
        LogError("Type mismatch: expected " + node.type->toString() + ", got " + node.expression->inferredType->toString());
    }
}

void SymanticVisitor::visit(ReturnNode &node)
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