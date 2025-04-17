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
    const std::shared_ptr<ASTNode>& expr)
{
    auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(expectedType);
    auto block = std::dynamic_pointer_cast<BlockNode>(expr);
    if (!genericType || !block) {
        LogError("Invalid collection initialization");
        return;
    }

    std::vector<std::string> keys;
    if (genericType->baseName == "array") {
        for (auto& statement : block->statements) {
            if (!statement) continue;
            // Для вложенных коллекций рекурсивно
            if (auto subBlock = std::dynamic_pointer_cast<BlockNode>(statement)) {
                validateCollectionElements(genericType->typeParameters[0], subBlock);
            } else {
                statement->accept(*this);
                auto elemType = statement->inferredType;
                auto expectedElemType = genericType->typeParameters[0];
                if (auto expectedGeneric = std::dynamic_pointer_cast<GenericTypeNode>(expectedElemType)) {
                    // Вложенная коллекция
                    validateCollectionElements(expectedElemType, statement);
                } else if (elemType->toString() != expectedElemType->toString()) {
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
                    validateCollectionElements(expectedKeyType, keyValue->key);
                } else if (keyType->toString() != expectedKeyType->toString()) {
                    LogError("Key type mismatch: expected " + expectedKeyType->toString() + ", got " + keyType->toString());
                }

                // Проверка значения
                if (auto expectedGenericValue = std::dynamic_pointer_cast<GenericTypeNode>(expectedValueType)) {
                    validateCollectionElements(expectedValueType, keyValue->value);
                } else if (valueType->toString() != expectedValueType->toString()) {
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
}