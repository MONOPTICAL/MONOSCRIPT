#include "../headers/TypeSymbolVisitor.h"
#include <algorithm>
#include <functional>

// Заглушки для visit-методов

void TypeSymbolVisitor::visit(AccessExpression& node) {
    lastAccessIndexedString = false;
    lastAccessResultField = false;
    std::shared_ptr<TypeNode> currentType = nullptr;
    std::shared_ptr<StructNode> currentStructDef = nullptr;
    std::shared_ptr<NamespaceNode> currentNamespaceDef = nullptr;       
    auto currentLink = node.shared_from_this();

    if (auto idNode = std::dynamic_pointer_cast<IdentifierNode>(node.expression))
        resolveNamespaceVariable(idNode->name);

    // math.f(1) сворачивается в вызов функции math.f, Xyi.x в переменную Xyi.x
    if (auto idNode = std::dynamic_pointer_cast<IdentifierNode>(node.expression);
        idNode && !contexts.back().variables.count(idNode->name) && registry.findNamespace(idNode->name)) {
        auto next = std::dynamic_pointer_cast<AccessExpression>(node.nextAccess);
        auto call = next ? std::dynamic_pointer_cast<CallNode>(next->expression) : nullptr;
        auto variable = next ? std::dynamic_pointer_cast<IdentifierNode>(next->expression) : nullptr;
        if (!call && !variable) {
            LogError("Only functions and variables can be accessed through namespace '" + idNode->name + "'", node.shared_from_this());
            return;
        }
        if (call) call->callee = idNode->name + "." + call->callee;
        else variable->name = idNode->name + "." + variable->name;
        node.baseName = call ? call->callee : variable->name;
        node.expression = next->expression;
        node.memberName = next->memberName;
        node.notation = next->notation;
        node.nextAccess = next->nextAccess;
        if (!node.nextAccess) {
            node.expression->accept(*this);
            node.inferredType = node.expression->inferredType;
            return;
        }
    }

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
                currentNamespaceDef = registry.findNamespace(idNode->name);
            }
        } else { 
            node.expression->accept(*this);
            currentType = node.expression->inferredType;
        }
    }

    if (currentType) {
        currentStructDef = registry.findStruct(currentType->toString());
        currentNamespaceDef = registry.findNamespace(currentType->toString());
    }

    if (!currentType && !currentStructDef) {
        LogError("Could not resolve base of chained access: '" + node.baseName + "'", node.shared_from_this());
        return;
    }

    while (currentLink) {
        auto accessNode = std::dynamic_pointer_cast<AccessExpression>(currentLink);
        if (!accessNode) break;

        if (!accessNode->memberName.empty() && currentType && currentType->toString() == "panic_info") {
            auto member = std::dynamic_pointer_cast<AccessExpression>(accessNode->nextAccess);
            const auto& name = accessNode->memberName;
            if (name != "message" && name != "line")
                LogError("panic_info has only message and line fields", accessNode);
            if (member && std::dynamic_pointer_cast<CallNode>(member->expression))
                LogError("panic_info members are fields, not methods", accessNode);
            currentType = registry.findType(name == "line" ? "i32" : "string");
            if (member) member->inferredType = currentType;
            currentStructDef = nullptr;
            lastAccessResultField = true;
            currentLink = accessNode->nextAccess;
            continue;
        }
        // Поля result: ok, code, error, value. Писать в них нельзя
        if (!accessNode->memberName.empty() && currentType && std::dynamic_pointer_cast<GenericTypeNode>(currentType)
            && std::dynamic_pointer_cast<GenericTypeNode>(currentType)->baseName == "result") {
            auto result = std::dynamic_pointer_cast<GenericTypeNode>(currentType);
            auto memberLink = std::dynamic_pointer_cast<AccessExpression>(accessNode->nextAccess);
            const std::string member = accessNode->memberName;
            std::shared_ptr<TypeNode> memberType;
            if (member == "ok") memberType = registry.findType("i1");
            else if (member == "code") memberType = registry.findType("i32");
            else if (member == "error") memberType = registry.findType("string");
            else if (member == "value") {
                if (result->typeParameters[0]->toString() == "void")
                    LogError("result<void> has no value", accessNode);
                memberType = result->typeParameters[0];
            }
            else LogError("result has no member '" + member + "', only ok, code, error and value", accessNode);

            if (memberLink && std::dynamic_pointer_cast<CallNode>(memberLink->expression))
                LogError("'" + member + "' of a result is a field, not a method", accessNode);
            if (memberLink) memberLink->inferredType = memberType;
            currentType = memberType;
            currentStructDef = registry.findStruct(currentType->toString());
            lastAccessResultField = true;
            currentLink = accessNode->nextAccess;
            continue;
        }

        if (!accessNode->memberName.empty()) {
            lastAccessResultField = false;
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
                    if (auto method = std::dynamic_pointer_cast<FunctionNode>(stmt)) return method->name == accessNode->memberName;
                    return false;
                });

            if (fieldIt == body->statements.end()) {
                LogError("Field '" + accessNode->memberName + "' not found in struct '" + currentStructDef->name + "'.", accessNode);
                return;
            }

            auto memberLink = std::dynamic_pointer_cast<AccessExpression>(accessNode->nextAccess);
            auto call = memberLink ? std::dynamic_pointer_cast<CallNode>(memberLink->expression) : nullptr;
            auto method = std::dynamic_pointer_cast<FunctionNode>(*fieldIt);
            if (call && !method)
                LogError("'" + accessNode->memberName + "' is a field of struct '" + currentStructDef->name + "', not a method", accessNode);
            if (method && !call)
                LogError("Method '" + accessNode->memberName + "' of struct '" + currentStructDef->name + "' must be called with ()", accessNode);

            const std::string structName = currentStructDef->name;
            currentType = nullptr;
            currentStructDef = nullptr;
            if (method) {
                const std::string methodName = structName + "." + method->name;
                const bool methodIsPure = std::find(method->labels.begin(), method->labels.end(), "@pure") != method->labels.end();

                if (checkLabels("@pure") && !methodIsPure)
                    LogError("Pure function cannot call impure method: " + methodName, accessNode);

                // const запрещает менять объект через эту переменную, а метод может его поменять
                if (!methodIsPure) {
                    if (auto baseId = std::dynamic_pointer_cast<IdentifierNode>(node.expression)) {
                        auto base = contexts.back().variables.find(baseId->name);
                        if (base != contexts.back().variables.end())
                            if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(base->second); var && var->isConst)
                                LogError("Constant '" + baseId->name + "' can only call @pure methods, not " + methodName, accessNode);
                    }
                }

                if (call->arguments.size() + 1 != method->parameters.size())
                    LogError("Method " + methodName + " expects " + std::to_string(method->parameters.size() - 1) + " arguments, got " + std::to_string(call->arguments.size()), accessNode);
                for (const auto& argument : call->arguments) argument->accept(*this);
                checkCallArguments(call->arguments, method->parameters, 1, "Method " + methodName);

                currentType = method->returnType;
                currentStructDef = registry.findStruct(currentType->toString());
                memberLink->inferredType = currentType;
            }
            else if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(*fieldIt)) {
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

            // s[i] это один байт строки
            if (currentType->toString() == "string") {
                auto indexNode = std::dynamic_pointer_cast<AccessExpression>(accessNode->nextAccess);
                if (indexNode && indexNode->expression) {
                    indexNode->expression->accept(*this);
                    auto indexType = indexNode->expression->implicitCastTo ? indexNode->expression->implicitCastTo
                                                                           : indexNode->expression->inferredType;
                    if (!indexType || getTypeRank(indexType->toString()) == 0 || isFloatType(indexType->toString()))
                        LogError("String index must be an integer, got " + (indexType ? indexType->toString() : std::string("unknown")), indexNode->expression);
                    if (resolveExprType(indexNode->expression).empty())
                        castAllNumbersToType(indexNode->expression, "i64");
                    indexNode->inferredType = registry.findType("char");
                }
                currentType = registry.findType("char");
                currentStructDef = nullptr;
                lastAccessIndexedString = true;
                currentLink = accessNode->nextAccess;
                continue;
            }
            lastAccessIndexedString = false;
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
                    if (rank == 0 || isFloatType(indexTypeStr))
                        LogError("Array index must be an integer, got " + indexTypeStr, indexNode->expression);
                    // Литерал 1 имеет тип i1, а -1 в i1 это снова 1
                    if (resolveExprType(indexNode->expression).empty())
                        castAllNumbersToType(indexNode->expression, "i64");
                } else {
                    const std::string keyTypeStr = genericType->typeParameters[0]->toString();
                    const bool bothIntegers = getTypeRank(indexTypeStr) > 0 && !isFloatType(indexTypeStr) &&
                                              getTypeRank(keyTypeStr) > 0 && !isFloatType(keyTypeStr);
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
        checkCharCast(node);
    } else if (currentStructDef) {
        LogError("Access expression resulted in a type ('" + currentStructDef->name + "'), not a value.", node.shared_from_this());
    }
}

void TypeSymbolVisitor::checkPureAlias(const std::shared_ptr<ASTNode>& expr) {
    if (!checkLabels("@pure") || !expr) return;
    std::function<bool(const std::shared_ptr<TypeNode>&)> mutableType;
    mutableType = [&](const std::shared_ptr<TypeNode>& type) {
        if (!type) return false;
        if (auto generic = std::dynamic_pointer_cast<GenericTypeNode>(type); generic && generic->baseName == "result") {
            for (const auto& parameter : generic->typeParameters)
                if (mutableType(parameter)) return true;
            return false;
        }
        const auto name = type->toString();
        return name.starts_with("array<") || name.starts_with("map<") || registry.findStruct(name) != nullptr;
    };
    auto nestedReferences = [&](const std::shared_ptr<TypeNode>& type) {
        if (auto generic = std::dynamic_pointer_cast<GenericTypeNode>(type)) {
            for (const auto& element : generic->typeParameters)
                if (mutableType(element)) return true;
        } else if (auto structure = registry.findStruct(type->toString())) {
            if (auto body = std::dynamic_pointer_cast<BlockNode>(structure->body))
                for (const auto& member : body->statements)
                    if (auto field = std::dynamic_pointer_cast<VariableAssignNode>(member))
                        if (mutableType(field->type)) return true;
        }
        return false;
    };
    std::set<const ASTNode*> visiting;
    std::function<bool(const std::shared_ptr<ASTNode>&)> borrowed;
    borrowed = [&](const std::shared_ptr<ASTNode>& value) -> bool {
        if (!value || !visiting.insert(value.get()).second) return false;
        bool result = false;
        if (auto block = std::dynamic_pointer_cast<BlockNode>(value)) {
            for (const auto& item : block->statements) result = borrowed(item) || result;
        } else if (auto pair = std::dynamic_pointer_cast<KeyValueNode>(value)) {
            result = borrowed(pair->value);
        } else if (mutableType(value->inferredType)) {
            if (auto id = std::dynamic_pointer_cast<IdentifierNode>(value)) {
                auto it = contexts.back().variables.find(id->name);
                if (it != contexts.back().variables.end())
                    if (auto variable = std::dynamic_pointer_cast<VariableAssignNode>(it->second))
                        result = !variable->expression || borrowed(variable->expression);
            } else if (auto access = std::dynamic_pointer_cast<AccessExpression>(value)) {
                result = borrowed(access->expression);
            } else if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(value)) {
                result = borrowed(unary->operand) || borrowed(unary->fallback);
            } else if (auto call = std::dynamic_pointer_cast<CallNode>(value)) {
                const bool builtinCopy = call->callee == "copy" &&
                    !contexts.back().functions.count("copy") && !declaredFunctions.count("copy");
                // Flat copies own all mutable storage. Nested copies still share it.
                if (!builtinCopy || nestedReferences(value->inferredType))
                    for (const auto& argument : call->arguments) result = borrowed(argument) || result;
            }
        }
        visiting.erase(value.get());
        return result;
    };
    if (borrowed(expr))
        LogError("Pure function cannot create a new reference to object passed to it; copy only detaches the outer object", expr);
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
    // m.x = 6: после свёртки namespace цель это сама переменная, и константу так не поменять
    if (auto target = std::dynamic_pointer_cast<IdentifierNode>(access->expression); target && !access->nextAccess) {
        auto it = contexts.back().variables.find(target->name);
        if (it != contexts.back().variables.end())
            if (auto var = std::dynamic_pointer_cast<VariableAssignNode>(it->second); var && var->isConst)
                LogError("Cannot reassign constant variable: " + target->name, node.shared_from_this());
    }
    checkPureAlias(access->expression);
    auto targetType = access->inferredType;
    if (!targetType)
        LogError("Could not resolve type of assignment target", node.accessExpression);
    if (lastAccessIndexedString)
        LogError("Strings are immutable, build a new string instead", node.accessExpression);
    if (lastAccessResultField)
        LogError("Fields of a result are read-only, return a new ok() or err() instead", node.accessExpression);
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