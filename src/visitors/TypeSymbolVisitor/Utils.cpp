#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::LogError(const std::string &message)
{
    throw std::runtime_error("Semantic Error: " + message);
}

std::shared_ptr<TypeNode> TypeSymbolVisitor::checkForIdentifier(std::shared_ptr<ASTNode>& node)
{
    if (!node) {
        LogError("Node is null");
    }

    if (auto idNode = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        auto it = contexts.back().variables.find(idNode->name);
        if (it == contexts.back().variables.end()) 
            LogError("Variable not found: " + idNode->name);

        auto varAssign = std::dynamic_pointer_cast<VariableAssignNode>(it->second);
        if (!varAssign) 
            LogError("!!!This doesn't need to happen!!!");

        if (varAssign->expression == nullptr) {
            if (!varAssign->inferredType)
                LogError("Variable '" + idNode->name + "' has no inferred type");
            return varAssign->inferredType;
        }

        if (!varAssign->expression->inferredType)
            LogError("Expression type is null for variable: " + idNode->name);

        return varAssign->expression->inferredType;
    }

    if (!node->inferredType)
        LogError("Node type is null");

    return node->inferredType;
}

void TypeSymbolVisitor::debugContexts()
{
    for (const auto& context : contexts) {
        std::cout << "Context: " << context.currentFunctionName << "\n";
        std::cout << "Variables:\n";
        for (const auto& var : context.variables) {
            std::cout << "  " << var.first << ": ";
            if (var.second) {
                auto typePtr = std::dynamic_pointer_cast<VariableAssignNode>(var.second);
                if (typePtr) {
                    std::cout << typePtr->name << " - ";
                    auto inferredType = typePtr->inferredType;
                    if (inferredType) {
                        std::cout << inferredType->toString() << "\n";
                    } else {
                        std::cout << "<null inferred type> ";
                        std::cout << typePtr->type->toString() << "\n";
                    }
                } else {
                    std::cout << "<null variable>\n";
                }
            } else {
                std::cout << "<null node>\n";
            }
        }
        std::cout << "Functions:\n";
        try {
            if (context.functions.empty()) {
                std::cout << "  <no functions>\n";
            } else {
                for (const auto& func : context.functions) {
                    if (func.first.empty()) {
                        std::cout << "  <unnamed function>\n";
                    } else {
                        std::cout << "  " << func.first << "\n";
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cout << "  <error accessing functions: " << e.what() << ">\n";
        } catch (...) {
            std::cout << "  <unknown error accessing functions>\n";
        }
    }

    std::cout << "---------------------\n";
}

void TypeSymbolVisitor::numCast(std::shared_ptr<ASTNode> &left, std::shared_ptr<ASTNode> &right, const std::string &op)
{
    if (!left) {
        LogError("Left node is nullptr in numCast");
        return;
    }
    if (!right) {
        LogError("Right node is nullptr in numCast");
        return;
    }

    auto Left = checkForIdentifier(left);
    auto Right = checkForIdentifier(right);
    
    if (!Left) {
        LogError("Left type is nullptr after checkForIdentifier");
        return;
    }
    if (!Right) {
        LogError("Right type is nullptr after checkForIdentifier");
        return;
    }
    
    auto leftType = Left->toString();
    auto rightType = Right->toString();
    IC(leftType, rightType);

    if (leftType == rightType) {
        return;
    }

    if (
        op == "+"
        || op == "-"
        || op == "*"
        || op == "/"
        || op == "%"
        || op == "=="
        || op == "!="
        || op == "<"
        || op == ">"
        || op == "<="
        || op == ">="
    ) {
        // Promote to the "larger" type if types differ and both are integer types
        auto getTypeRank = [](const std::string& type) -> int {
            if (type == "i1") return 1;
            if (type == "i8") return 2;
            if (type == "i32") return 3;
            if (type == "i64") return 4;
            return 0; // unknown type
        };

        int leftRank = getTypeRank(leftType);
        int rightRank = getTypeRank(rightType);
        
        if (leftRank > 0 && rightRank > 0 && leftType != rightType) {
            // Promote the node with the lower type rank to the higher type
            std::string targetType = (leftRank > rightRank) ? leftType : rightType;
            if (leftRank < rightRank) {
            left->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
            } else if (rightRank < leftRank) {
            right->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
            }
            left->inferredType = std::make_shared<SimpleTypeNode>(targetType);
            right->inferredType = std::make_shared<SimpleTypeNode>(targetType);
            return;
        }
    }

    LogError("Unsupported operand types for " + op + " : " + leftType + " and " + rightType);
}

void TypeSymbolVisitor::validateCollectionElements(
    const std::shared_ptr<TypeNode>& expectedType,
    const std::shared_ptr<ASTNode>& expr,
    bool isAuto)
{
    auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(expectedType);
    auto block = std::dynamic_pointer_cast<BlockNode>(expr);
    if (!genericType || !block) 
    {
        if (std::dynamic_pointer_cast<NoneNode>(expr))
            return;

        LogError("Invalid collection initialization");
        return;
    }
    std::shared_ptr<ASTNode> lastStatement = block->statements[0];
    std::vector<std::string> keys;

    int maxRank = 0;
    auto numericRank = [](const std::string& type, int& maxRank) -> int {
        if (type == "i1") { if (maxRank < 1) maxRank = 1; return 1; }
        if (type == "i8") { if (maxRank < 2) maxRank = 2; return 2; }
        if (type == "i32") { if (maxRank < 3) maxRank = 3; return 3; }
        if (type == "i64") { if (maxRank < 4) maxRank = 4; return 4; }
        if (type == "float") { if (maxRank < 5) maxRank = 5; return 5; }
        return 0; // unknown type
    };

    if (genericType->baseName == "array") {
        for (auto& statement : block->statements) {
            if (!statement) continue;
            // Для вложенных коллекций рекурсивно
            if (auto subBlock = std::dynamic_pointer_cast<BlockNode>(statement)) {
                validateCollectionElements(genericType->typeParameters[0], subBlock, isAuto);
            } else {
                statement->accept(*this);
                auto elemType = statement->inferredType;
                auto expectedElemType = genericType->typeParameters[0];
                if (auto expectedGeneric = std::dynamic_pointer_cast<GenericTypeNode>(expectedElemType)) {
                    // Вложенная коллекция
                    validateCollectionElements(expectedElemType, statement, isAuto);
                } else if (elemType->toString() != expectedElemType->toString()) {
                    if ((numericRank(elemType->toString(), maxRank) > 0 && numericRank(expectedElemType->toString(), maxRank) > 0))
                        continue;

                    LogError("Element type mismatch: expected " + expectedElemType->toString() + ", got " + elemType->toString());
                }
            }
        }
    } else if (genericType->baseName == "map") {
        for (auto& statement : block->statements) {
            if (!statement) continue;
            if (auto keyValue = std::dynamic_pointer_cast<KeyValueNode>(statement)) {
                keyValue->key->accept(*this);
                keyValue->value->accept(*this);

                auto keyType = keyValue->key->inferredType;
                auto valueType = keyValue->value->inferredType;
                auto expectedKeyType = genericType->typeParameters[0];
                auto expectedValueType = genericType->typeParameters[1];

                // Проверка ключа
                if (auto expectedGenericKey = std::dynamic_pointer_cast<GenericTypeNode>(expectedKeyType)) {
                    validateCollectionElements(expectedKeyType, keyValue->key, isAuto);
                } else if (keyType->toString() != expectedKeyType->toString()) {
                    if (!(numericRank(keyType->toString(), maxRank) > 0 && numericRank(expectedKeyType->toString(), maxRank) > 0))
                        LogError("Key type mismatch: expected " + expectedKeyType->toString() + ", got " + keyType->toString());
                }

                // Проверка значения
                if (auto expectedGenericValue = std::dynamic_pointer_cast<GenericTypeNode>(expectedValueType)) {
                    validateCollectionElements(expectedValueType, keyValue->value, isAuto);
                } else if (valueType->toString() != expectedValueType->toString()) {
                    if (!(numericRank(valueType->toString(), maxRank) > 0 && numericRank(expectedValueType->toString(), maxRank) > 0))   
                        LogError("Value type mismatch: expected " + expectedValueType->toString() + ", got " + valueType->toString());
                }

                // Проверка ключей на дубликацию
                if (std::find(keys.begin(), keys.end(), keyValue->keyName) != keys.end()) {
                    LogError("Duplicate key: " + keyValue->key->inferredType->toString());
                }
                keys.push_back(keyValue->keyName);
            } else {
                statement->accept(*this);
                LogError("Map initialization expects key-value pairs");
            }
        }
    } else {
        LogError("Unknown generic collection: " + genericType->baseName);
    }
    
    if (maxRank > 0) {
        applyImplicitCastToNumeric(expectedType, block, maxRank, isAuto);
    }
}

void TypeSymbolVisitor::applyImplicitCastToNumeric(
    const std::shared_ptr<TypeNode>& expectedType,
    const std::shared_ptr<ASTNode>& expr,
    int maxRank,
    bool isAuto)
{
    auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(expectedType);
    auto block = std::dynamic_pointer_cast<BlockNode>(expr);
    if (!genericType || !block) return;

    // Определяем целевой тип по maxRank (для auto)
    std::string targetType;
    switch (maxRank) {
        case 1: targetType = "i1"; break;
        case 2: targetType = "i8"; break;
        case 3: targetType = "i32"; break;
        case 4: targetType = "i64"; break;
        case 5: targetType = "float"; break;
        default: return;
    }

    // Если auto, меняем genericType, если нет — используем только текущий genericType
    if (isAuto) {
        if (genericType->baseName == "array" || genericType->baseName == "map") {
            for (size_t i = 0; i < genericType->typeParameters.size(); ++i) {
                auto param = genericType->typeParameters[i];
                if (auto simple = std::dynamic_pointer_cast<SimpleTypeNode>(param)) {
                    if (
                        simple->toString() == "i1" ||
                        simple->toString() == "i8" ||
                        simple->toString() == "i32" ||
                        simple->toString() == "i64" ||
                        simple->toString() == "float"
                    ) {
                        if (simple->toString() != targetType) {
                            genericType->typeParameters[i] = std::make_shared<SimpleTypeNode>(targetType);
                        }
                    }
                } else if (auto nestedGeneric = std::dynamic_pointer_cast<GenericTypeNode>(param)) {
                    applyImplicitCastToNumeric(nestedGeneric, expr, maxRank, isAuto);
                }
            }
        }
    } else {
        // Если не auto, targetType = текущий genericType
        if (!genericType->typeParameters.empty()) {
            if (auto simple = std::dynamic_pointer_cast<SimpleTypeNode>(genericType->typeParameters[0])) {
                targetType = simple->toString();
            }
        }
    }

    // Проверка лимитов для не-auto
    auto checkLimits = [](const std::string& generic, int value) -> bool {
        if (generic == "i1") return value == 0 || value == 1;
        if (generic == "i8") return value >= -128 && value <= 127;
        if (generic == "i32") return value >= -32768 && value <= 32767;
        if (generic == "i64") return value >= -2147483648LL && value <= 2147483647LL;
        return true; // float и др. не проверяем, або нахуй надо
    };

    if (genericType->baseName == "array") {
        for (auto& statement : block->statements) {
            if (!statement) continue;
            if (auto subBlock = std::dynamic_pointer_cast<BlockNode>(statement)) {
                applyImplicitCastToNumeric(genericType->typeParameters[0], subBlock, maxRank, isAuto);
            } else if (auto numNode = std::dynamic_pointer_cast<NumberNode>(statement)) {
                auto elemType = numNode->inferredType;
                int value = numNode->value;
                if (!isAuto) {
                    // Проверяем лимиты
                    if (!checkLimits(targetType, value)) {
                        LogError("Element value " + std::to_string(value) + " does not fit in type " + targetType);
                    }
                }
                if (elemType && elemType->toString() != targetType) {
                    numNode->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
                }
            }
        }
    } else if (genericType->baseName == "map") {
        for (auto& statement : block->statements) {
            if (!statement) continue;
            if (auto keyValue = std::dynamic_pointer_cast<KeyValueNode>(statement)) {
                // Ключ
                if (auto numNode = std::dynamic_pointer_cast<NumberNode>(keyValue->key)) {
                    int value = numNode->value;
                    std::string keyTargetType = targetType;
                    if (!isAuto && !genericType->typeParameters.empty()) {
                        if (auto simple = std::dynamic_pointer_cast<SimpleTypeNode>(genericType->typeParameters[0]))
                            keyTargetType = simple->toString();
                    }
                    if (!isAuto && !checkLimits(keyTargetType, value)) {
                        LogError("Key value " + std::to_string(value) + " does not fit in type " + keyTargetType);
                    }
                    if (numNode->inferredType && numNode->inferredType->toString() != keyTargetType) {
                        numNode->implicitCastTo = std::make_shared<SimpleTypeNode>(keyTargetType);
                    }
                }
                // Значение (может быть array или map)
                if (auto subBlock = std::dynamic_pointer_cast<BlockNode>(keyValue->value)) {
                    std::shared_ptr<TypeNode> valueType = genericType->typeParameters.size() > 1 ? genericType->typeParameters[1] : nullptr;
                    applyImplicitCastToNumeric(valueType, subBlock, maxRank, isAuto);
                } else if (auto numNode = std::dynamic_pointer_cast<NumberNode>(keyValue->value)) {
                    int value = numNode->value;
                    std::string valueTargetType = targetType;
                    if (!isAuto && genericType->typeParameters.size() > 1) {
                        if (auto simple = std::dynamic_pointer_cast<SimpleTypeNode>(genericType->typeParameters[1]))
                            valueTargetType = simple->toString();
                    }
                    if (!isAuto && !checkLimits(valueTargetType, value)) {
                        LogError("Value " + std::to_string(value) + " does not fit in type " + valueTargetType);
                    }
                    if (numNode->inferredType && numNode->inferredType->toString() != valueTargetType) {
                        numNode->implicitCastTo = std::make_shared<SimpleTypeNode>(valueTargetType);
                    }
                }
            }
        }
    }
}