#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::visit(SimpleTypeNode &node)
{
    // Проверяем, существует ли тип в реестре
    auto type = registry.findType(node.name);
    if (!type && !registry.findStruct(node.name)) {
        LogError("Unknown type: " + node.name);
    }

    // Структуры в реестре типов нет, тип такого узла это он сам
    node.inferredType = type ? type : std::make_shared<SimpleTypeNode>(node.name);
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
    
    // Ключ map сравнивается на равенство: у float мешает точность, а у объектов равенства нет
    if (node.baseName == "map" && !node.typeParameters.empty()) {
        const std::string keyType = node.typeParameters[0]->toString();
        if (keyType != "string" && keyType != "char" && (getTypeRank(keyType) == 0 || isFloatType(keyType)))
            LogError("Map key must be an integer, char or string, got " + keyType);
    }

    node.inferredType = baseType;
}

void TypeSymbolVisitor::checkFinalAssigned(const std::shared_ptr<ASTNode>& variable, const std::string& name, const std::shared_ptr<ASTNode>& usage)
{
    auto final = std::dynamic_pointer_cast<VariableAssignNode>(variable);
    if (final && final->isDeferred && assignedFinals.count(variable.get()) == 0)
        LogError("Final variable '" + name + "' is used before it is assigned", usage);
}

void TypeSymbolVisitor::visit(IdentifierNode& node)
{
    // Проверяем, существует ли переменная в реестре
    auto it = contexts.back().variables.find(node.name);
    if (it == contexts.back().variables.end()) {
        LogError("Variable not found: " + node.name, node.shared_from_this());
    }

    std::shared_ptr<ASTNode> varAssign = contexts.back().variables[node.name];
    checkFinalAssigned(varAssign, node.name, node.shared_from_this());
    auto result = checkForIdentifier(varAssign);

    if(varAssign->implicitCastTo)
    {
        node.inferredType = varAssign->implicitCastTo;
    }
    node.inferredType = result;
    checkCharCast(node);
}

// char не число: неявно не приводится, а явно только в целое и из целого
void TypeSymbolVisitor::checkCharCast(ASTNode& node)
{
    if (!node.inferredType || !node.implicitCastTo) return;
    const std::string from = node.inferredType->toString(), to = node.implicitCastTo->toString();
    if (from == to) return;

    const bool integerFrom = getTypeRank(from) > 0 && !isFloatType(from);
    const bool integerTo = getTypeRank(to) > 0 && !isFloatType(to);
    if ((to == "char" && !integerFrom) || (from == "char" && !integerTo))
        LogError("Cannot cast " + from + " to " + to, node.shared_from_this());
}

void TypeSymbolVisitor::visit(NumberNode& node)
{
    node.inferredType = registry.findType(node.type->toString()); // Тут пока что i32 потом разделим на i8, i32, i64
    checkCharCast(node);
}

void TypeSymbolVisitor::visit(FloatNumberNode& node)
{
    node.inferredType = registry.findType("f64"); // Без указания типа самый точный, x ^= 0.1 это f64
    checkCharCast(node);
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

void TypeSymbolVisitor::visit(ModuleMark &node)
{
    this->currentModuleName = node.moduleName;
}