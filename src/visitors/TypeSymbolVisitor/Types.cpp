#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::visit(SimpleTypeNode &node)
{
    // Проверяем, существует ли тип в реестре
    auto type = registry.findType(node.name);
    if (!type) {
        LogError("Unknown type: " + node.name);
    }
    node.inferredType = type;
}

void TypeSymbolVisitor::visit(GenericTypeNode &node)
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

void TypeSymbolVisitor::visit(IdentifierNode& node)
{
    // Проверяем, существует ли переменная в реестре
    auto it = contexts.back().variables.find(node.name);
    if (it == contexts.back().variables.end()) {
        LogError("Variable not found: " + node.name);
    }

    std::shared_ptr<ASTNode> varAssign = contexts.back().variables[node.name];
    auto result = checkForIdentifier(varAssign);
    node.inferredType = result;
}

void TypeSymbolVisitor::visit(NumberNode& node)
{
    node.inferredType = registry.findType(node.type->toString()); // Тут пока что i32 потом разделим на i8, i32, i64
}

void TypeSymbolVisitor::visit(FloatNumberNode& node)
{
    node.inferredType = registry.findType("float");
}

void TypeSymbolVisitor::visit(StringNode& node)
{
    node.inferredType = registry.findType("string");
}

void TypeSymbolVisitor::visit(NullNode& node)
{
    node.inferredType = registry.findType("null");
}

void TypeSymbolVisitor::visit(NoneNode& node)
{
    node.inferredType = registry.findType("none");
}

void TypeSymbolVisitor::visit(KeyValueNode& node) {
    node.key->accept(*this);
    node.value->accept(*this);
    auto keyType = checkForIdentifier(node.key);
    auto valueType = checkForIdentifier(node.value);
}
