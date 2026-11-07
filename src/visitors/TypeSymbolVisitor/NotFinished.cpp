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
                    idNode->inferredType = currentType; // С этого типа кодген начинает обход цепочки
                    checkFinalAssigned(varIt->second, idNode->name, node.shared_from_this());
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
                // Кодгену нужен тип на каждом звене: сам он типы не выводит
                if (auto fieldLink = std::dynamic_pointer_cast<AccessExpression>(accessNode->nextAccess))
                    fieldLink->inferredType = currentType;
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

            // Индекс лежит в следующем звене цепочки
            auto indexNode = std::dynamic_pointer_cast<AccessExpression>(accessNode->nextAccess);
            if (indexNode && indexNode->expression) {
                indexNode->expression->accept(*this);
                auto indexType = indexNode->expression->implicitCastTo ? indexNode->expression->implicitCastTo
                                                                       : indexNode->expression->inferredType;
                if (!indexType) {
                    LogError("Could not resolve type of index", indexNode->expression);
                    return;
                }

                const std::string indexTypeStr = indexType->toString();
                if (genericType->baseName == "array") {
                    const int rank = getTypeRank(indexTypeStr);
                    if (rank == 0 || indexTypeStr == "float")
                        LogError("Array index must be an integer, got " + indexTypeStr, indexNode->expression);
                    // Литерал 1 имеет тип i1, а -1 в i1 это снова 1
                    if (resolveExprType(indexNode->expression).empty())
                        castAllNumbersToType(indexNode->expression, "i64");
                } else {
                    const std::string keyTypeStr = genericType->typeParameters[0]->toString();
                    const bool bothIntegers = getTypeRank(indexTypeStr) > 0 && indexTypeStr != "float" &&
                                              getTypeRank(keyTypeStr) > 0 && keyTypeStr != "float";
                    if (indexTypeStr != keyTypeStr && !bothIntegers)
                        LogError("Map key must be " + keyTypeStr + ", got " + indexTypeStr, indexNode->expression);
                    // Сужение обрезало бы ключ, и нашлось бы не то значение
                    checkConversion(indexNode->expression, keyTypeStr);
                }
            }

            currentType =(genericType->baseName == "array") ? genericType->typeParameters[0] : genericType->typeParameters[1];
            // Кодгену нужен тип на каждом звене: сам он типы не выводит
            if (indexNode) indexNode->inferredType = currentType;
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

void TypeSymbolVisitor::checkPureAlias(const std::shared_ptr<ASTNode>& expr) {
    if (!checkLabels("@pure") || !expr->inferredType) return;

    // Числа и строки не меняются на месте, опасны только объекты
    const std::string type = expr->inferredType->toString();
    if (!type.starts_with("array") && !type.starts_with("map") && !registry.findStruct(type)) return;

    std::shared_ptr<ASTNode> base = expr;
    if (auto access = std::dynamic_pointer_cast<AccessExpression>(expr)) base = access->expression;

    auto id = std::dynamic_pointer_cast<IdentifierNode>(base);
    if (!id) return;

    auto it = contexts.back().variables.find(id->name);
    if (it == contexts.back().variables.end()) return;

    if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(it->second); var && !var->expression)
        LogError("Pure function cannot create a new reference to object passed to it, use copy(" + id->name + ")", expr);
}

void TypeSymbolVisitor::visit(ReassignMemberNode& node) {
    auto access = std::dynamic_pointer_cast<AccessExpression>(node.accessExpression);
    if (!access)
        LogError("Left side of member assignment is not an access expression", node.shared_from_this());

    // Шоб не могли нихуя вставлять в результат вызова x.sum() = 1
    // В a[f()] = 1 вызов это индекс, а не цель присваивания
    auto last = access;
    bool lastIsIndex = false;
    while (auto next = std::dynamic_pointer_cast<AccessExpression>(last->nextAccess)) {
        lastIsIndex = last->notation == "[]";
        last = next;
    }
    if (!lastIsIndex && std::dynamic_pointer_cast<CallNode>(last->expression))
        LogError("Cannot assign to the result of a function call", node.accessExpression);

    // Шоб не меняли константы а то совсем ахуели
    if (auto baseId = std::dynamic_pointer_cast<IdentifierNode>(access->expression)) {
        auto it = contexts.back().variables.find(baseId->name);
        if (it != contexts.back().variables.end())
            if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(it->second); var && var->isConst)
                LogError("Cannot modify member of constant variable: " + baseId->name, node.shared_from_this());

        // У параметров и переменной цикла нет своего значения: это объект, который передали снаружи
        if (it != contexts.back().variables.end() && checkLabels("@pure"))
            if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(it->second); var && !var->expression)
                LogError("Pure function cannot modify object passed to it: " + baseId->name, node.shared_from_this());
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
    checkPureAlias(node.expression);
    checkConversion(node.expression, targetTypeStr);

    node.inferredType = targetType;
}

void TypeSymbolVisitor::visit(ImportNode &node)
{
    // Не надо его реализовывать, он обрабатывается в другом месте(при загрузке модулей в AST)
}