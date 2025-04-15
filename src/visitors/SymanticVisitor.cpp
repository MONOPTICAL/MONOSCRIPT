#include "headers/SymanticVisitor.h"

void SymanticVisitor::LogError(const std::string &message)
{
    throw std::runtime_error("Semantic Error: " + message);
}

void SymanticVisitor::visit(SimpleTypeNode &node)
{
    // Проверяем, существует ли тип в реестре
    auto type = registry.findType(node.name);
    if (!type) {
        LogError("Unknown type: " + node.name);
    }
    node.inferredType = type;
}

void SymanticVisitor::visit(GenericTypeNode &node)
{
    // Проверяем, существует ли базовый тип в реестре
    auto baseType = registry.findType(node.baseName);
    if (!baseType) {
        LogError("Unknown base type: " + node.baseName);
    }
    
    // Проверяем, что все параметры типа также известны
    for (const auto& param : node.typeParameters) {
        param->accept(*this);
    }
    
    node.inferredType = baseType;
}

void SymanticVisitor::visit(ProgramNode &node)
{
    for (const auto& statement : node.body) {
        statement->accept(*this);
    }
}

void SymanticVisitor::visit(FunctionNode &node)
{
    // Проверяем, существует ли функция в реестре
    auto func = registry.findFunction(node.name);
    if (!func) {
        LogError("Unknown function: " + node.name);
        this->currentScopeFunctions[node.name] = node.
    }
    
    // Проверяем, что все параметры функции известны
    for (const auto& param : node.parameters) {
        param.first->accept(*this); // Проверяем тип параметра
    }
    
    node.inferredType = func;
}