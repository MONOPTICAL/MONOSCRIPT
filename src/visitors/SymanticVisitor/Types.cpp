#include "../headers/SymanticVisitor.h"

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

void SymanticVisitor::visit(IdentifierNode& node)
{
    // Проверяем, существует ли переменная в реестре
    auto it = contexts.back().variables.find(node.name);
    if (it == contexts.back().variables.end()) {
        LogError("Variable not found: " + node.name);
    }

    this->debugContexts();
    IC();
    node.inferredType = checkForIdentifier(it->second);
}

void SymanticVisitor::visit(NumberNode& node)
{
    node.inferredType = registry.findType("i32"); // Тут пока что i32 потом разделим на i8, i32, i64
    IC(
        node.value,
        node.inferredType->toString()
    );
}

void SymanticVisitor::visit(FloatNumberNode& node)
{
    node.inferredType = registry.findType("float");
}

void SymanticVisitor::visit(StringNode& node)
{
    node.inferredType = registry.findType("string");
}

void SymanticVisitor::visit(BooleanNode& node)
{
    node.inferredType = registry.findType("bool");
}

void SymanticVisitor::visit(NullNode& node)
{
    node.inferredType = registry.findType("null");
}

void SymanticVisitor::visit(NoneNode& node)
{
    node.inferredType = registry.findType("none");
}


