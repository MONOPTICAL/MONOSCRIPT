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
    
    // Проверяем, что все параметры типа также известны. У func последний это результат, он может быть void
    for (size_t i = 0; i < node.typeParameters.size(); i++) {
        // void можно только там, где значения может не быть: результат func и result<void>
        const bool voidAllowed = (node.baseName == "func" && i + 1 == node.typeParameters.size()) || node.baseName == "result";
        if (voidAllowed && node.typeParameters[i]->toString() == "void") continue;
        node.typeParameters[i]->accept(*this);
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

std::string moduleNameOf(const std::string& path);

// Тип функции как значения: func<параметры..., результат>
std::shared_ptr<TypeNode> functionTypeOf(const FunctionNode& function) {
    auto type = std::make_shared<GenericTypeNode>("func");
    for (const auto& parameter : function.parameters) type->typeParameters.push_back(parameter.first);
    type->typeParameters.push_back(function.returnType ? function.returnType : std::make_shared<SimpleTypeNode>("void"));
    return type;
}

void TypeSymbolVisitor::visit(IdentifierNode& node)
{
    // Проверяем, существует ли переменная в реестре
    auto it = contexts.back().variables.find(node.name);
    if (it == contexts.back().variables.end()) {
        // Не переменная, а имя функции: её передают как значение
        std::shared_ptr<FunctionNode> function;
        std::string internalName = node.name;
        auto scope = moduleScopes.find(moduleNameOf(currentModuleName));
        if (scope != moduleScopes.end()) {
            auto visible = scope->second.find(node.name);
            if (visible != scope->second.end()) internalName = visible->second->name;
        }
        if (auto local = findLocalFunction(contexts.back(), node.name)) {
            function = local;
            internalName = local->name;
        }
        else if (auto declared = declaredFunctions.find(internalName); declared != declaredFunctions.end())
            function = std::dynamic_pointer_cast<FunctionNode>(declared->second);
        else if (auto builtin = registry.findFunction(node.name))
            function = std::dynamic_pointer_cast<FunctionNode>(builtin);

        if (!function) {
            LogError("Variable not found: " + node.name, node.shared_from_this());
        }
        if (!function->typeParameters.empty()) {
            LogError("Generic function " + node.name + " cannot be passed as a value yet", node.shared_from_this());
        }

        node.name = internalName;
        node.referencedFunction = function;
        node.inferredType = functionTypeOf(*function);
        return;
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
    if (node.value.find('\0') != std::string::npos)
        LogError("Strings cannot contain a NUL byte", node.shared_from_this());
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