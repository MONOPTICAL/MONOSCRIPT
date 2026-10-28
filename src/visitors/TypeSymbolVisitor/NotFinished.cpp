#include "../headers/TypeSymbolVisitor.h"
#include <algorithm>

// Заглушки для visit-методов

void TypeSymbolVisitor::visit(AccessExpression& node) {
    std::shared_ptr<TypeNode> currentType = nullptr;
    std::shared_ptr<StructNode> currentStructDef = nullptr;
    auto currentLink = node.shared_from_this();

    if (node.expression) {
        if (auto idNode = std::dynamic_pointer_cast<IdentifierNode>(node.expression)) {
            auto varIt = contexts.back().variables.find(idNode->name);
            if (varIt != contexts.back().variables.end()) {
                if (auto varAssign = std::dynamic_pointer_cast<VariableAssignNode>(varIt->second)) {
                    currentType = varAssign->type;
                }
            } else {
                currentStructDef = registry.findStruct(idNode->name);
            }
        } else { 
            node.expression->accept(*this);
            currentType = node.expression->inferredType;
        }
    }

    if (currentType) {
        currentStructDef = registry.findStruct(currentType->toString());
    }

    if (!currentType && !currentStructDef) {
        LogError("Could not resolve base of chained access: '" + node.baseName + "'", node.shared_from_this());
        return;
    }

    while (currentLink) {
        auto accessNode = std::dynamic_pointer_cast<AccessExpression>(currentLink);
        if (!accessNode) break;

        if (!accessNode->memberName.empty()) {
            if (!currentStructDef) {
                std::string typeName = currentType ? currentType->toString() : "unknown";
                LogError("Dot access on non-struct type '" + typeName + "'", accessNode);
                return;
            }
            
            auto body = std::dynamic_pointer_cast<BlockNode>(currentStructDef->body);
            if (!body) { return; }

            auto fieldIt = std::find_if(body->statements.begin(), body->statements.end(),
                [&](const auto& stmt) {
                    if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(stmt)) return var->name == accessNode->memberName;
                    if (auto str = std::dynamic_pointer_cast<StructNode>(stmt)) return str->name == accessNode->memberName;
                    return false;
                });

            if (fieldIt == body->statements.end()) {
                LogError("Field '" + accessNode->memberName + "' not found in struct '" + currentStructDef->name + "'.", accessNode);
                return;
            }

            currentType = nullptr;
            currentStructDef = nullptr;
            if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(*fieldIt)) {
                currentType = var->type;
                currentStructDef = registry.findStruct(currentType->toString()); 
            } else if (auto str = std::dynamic_pointer_cast<StructNode>(*fieldIt)) {
                currentStructDef = str;
            }
        }

        if (accessNode->notation == "[]") {
            if (!currentType) {
                 LogError("Index access '[]' attempted on a static type, not an instance.", accessNode);
                 return;
            }
            auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(currentType);
            if (!genericType || (genericType->baseName != "array" && genericType->baseName != "map")) {
                LogError("Index access '[]' attempted on non-collection type '" + currentType->toString() + "'.", accessNode);
                return;
            }
            currentType = (genericType->baseName == "array") ? genericType->typeParameters[0] : genericType->typeParameters[1];
            currentStructDef = registry.findStruct(currentType->toString());
        }
        
        currentLink = accessNode->nextAccess;
    }

    if (currentType) {
        node.inferredType = currentType;
    } else if (currentStructDef) {
        LogError("Access expression resulted in a type ('" + currentStructDef->name + "'), not a value.", node.shared_from_this());
    }
}

void TypeSymbolVisitor::visit(ReassignMemberNode& node) {
    auto access = std::dynamic_pointer_cast<AccessExpression>(node.accessExpression);
    if (!access)
        LogError("Left side of member assignment is not an access expression", node.shared_from_this());

    // Шоб не могли нихуя вставлять в результат вызова x.sum() = 1
    auto last = access;
    while (auto next = std::dynamic_pointer_cast<AccessExpression>(last->nextAccess))
        last = next;
    if (std::dynamic_pointer_cast<CallNode>(last->expression))
        LogError("Cannot assign to the result of a function call", node.accessExpression);

    // Шоб не меняли константы а то совсем ахуели
    if (auto baseId = std::dynamic_pointer_cast<IdentifierNode>(access->expression)) {
        auto it = contexts.back().variables.find(baseId->name);
        if (it != contexts.back().variables.end())
            if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(it->second); var && var->isConst)
                LogError("Cannot modify member of constant variable: " + baseId->name, node.shared_from_this());
    }

    access->accept(*this);
    auto targetType = access->inferredType;
    if (!targetType)
        LogError("Could not resolve type of assignment target", node.accessExpression);
    std::string targetTypeStr = targetType->toString();

    if (std::dynamic_pointer_cast<BlockNode>(node.expression)) {
        node.expression->accept(*this);
        validateCollectionElements(targetType, node.expression, false);
        node.inferredType = targetType;
        return;
    }

    node.expression->accept(*this);
    auto exprType = node.expression->implicitCastTo ? node.expression->implicitCastTo
                                                    : node.expression->inferredType;
    if (!exprType)
        LogError("Cannot infer expression type", node.expression);
    std::string exprTypeStr = exprType->toString();

    bool targetNumeric = getTypeRank(targetTypeStr) > 0;
    bool exprNumeric   = getTypeRank(exprTypeStr) > 0;

    if (!targetNumeric || !exprNumeric) {
        if (exprTypeStr != targetTypeStr)
        {
            IC("ZDES");
            LogError("Type mismatch: expected " + targetTypeStr + ", got " + exprTypeStr, node.expression);
        }
        node.inferredType = targetType;
        return;
    }

    // Проверка на i1 шоб хуйни не подсовывали 
    auto isBoolExpr = [](const std::shared_ptr<ASTNode>& e) {
        if (auto bin = std::dynamic_pointer_cast<NumberNode>(e))
            return bin->inferredType && bin->inferredType->toString() == "i1";
        return std::dynamic_pointer_cast<UnaryOpNode>(e) != nullptr;
    };

    IC(node.expression);
    if (isBoolExpr(node.expression) != (targetTypeStr == "i1"))
        LogError("Type mismatch: expected " + targetTypeStr + ", got " +
                 (isBoolExpr(node.expression) ? std::string("i1") : exprTypeStr), node.expression);

    if (checkLabels("@strict")) {
        if (exprTypeStr != targetTypeStr)
            LogError("Type mismatch in strict mode: expected " + targetTypeStr + ", got " + exprTypeStr, node.expression);
    }
    else if (!isBoolExpr(node.expression)) {
        castAndValidate(node.expression, targetTypeStr, this);

        auto after = node.expression->implicitCastTo ? node.expression->implicitCastTo
                                                     : node.expression->inferredType;
        if (after->toString() != targetTypeStr)
            node.expression->implicitCastTo = targetType;
    }

    node.inferredType = targetType;
}

void TypeSymbolVisitor::visit(ImportNode &node)
{
    // Не надо его реализовывать, он обрабатывается в другом месте(при загрузке модулей в AST)
}