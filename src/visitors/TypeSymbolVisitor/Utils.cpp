#include "../headers/TypeSymbolVisitor.h"
#include "../../includes/ASTDebugger.hpp"
void TypeSymbolVisitor::LogError(const std::string &message)
{
    std::cout << "--- ERROR ---\n" << std::endl;
    ASTDebugger::debug(this->program);
    std::cout << "-------------\n";
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

        if (varAssign->expression && varAssign->expression->implicitCastTo) 
            std::cout << varAssign->implicitCastTo->toString() << std::endl;
        return varAssign->expression->inferredType;
    }
    else if (auto varAssignNode = std::dynamic_pointer_cast<VariableAssignNode>(node)) {
        if (varAssignNode->expression == nullptr) {
            if (!varAssignNode->inferredType)
                LogError("Variable '" + varAssignNode->name + "' has no inferred type");
            if (varAssignNode->expression && varAssignNode->expression->implicitCastTo)
                std::cout << varAssignNode->implicitCastTo->toString() << std::endl;
            return varAssignNode->inferredType;
        }

        if (!varAssignNode->expression->inferredType)
            LogError("Expression type is null for variable: " + varAssignNode->name);

        return varAssignNode->expression->inferredType;
    }

    if (!node->inferredType)
        LogError("Node type is null");

    return node->inferredType;
}

void TypeSymbolVisitor::debugContexts()
{
    for (const auto& context : contexts) {
        std::cout << "---------------------\n";
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

static int getTypeRank(const std::string& type) {
    if (type == "i1") return 1;
    if (type == "i8") return 2;
    if (type == "i16") return 3;
    if (type == "i32") return 4;
    if (type == "i64") return 5;
    if (type == "float") return 6;
    return 0;
}

static std::string getTypeByRank(int rank) {
    switch (rank) {
        case 1: return "i1";
        case 2: return "i8";
        case 3: return "i16";
        case 4: return "i32";
        case 5: return "i64";
        case 6: return "float";
        default: return "";
    }
}

static std::string getOperation(const std::string& op, const std::string& type) {
    if (type == "float") {
        if (op == "add") return "fadd";
        if (op == "sub") return "fsub";
        if (op == "mul") return "fmul";
        if (op == "sdiv") return "fdiv";
        if (op == "srem") return "frem";
        if (op.rfind("icmp_", 0) == 0) return "fcmp_" + op.substr(5);
    }

    return op;
}

// Рекурсивно выставляет implicitCastTo для всех чисел, если их тип меньше чем targetType
static void castAllNumbersToType(const std::shared_ptr<ASTNode>& node, const std::string& targetType) {
    if (!node) return;
    if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
        std::string fromType = num->inferredType ? num->inferredType->toString() : (num->type ? num->type->toString() : "");
        if (fromType != targetType) {
            num->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
        }
        return;
    }
    if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        if (ident->inferredType->toString() != targetType)
            if (getTypeRank(ident->inferredType->toString()) > 0)
                ident->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        castAllNumbersToType(bin->left, targetType);
        castAllNumbersToType(bin->right, targetType);
        std::string implicitCast;

        if (bin->left->implicitCastTo) {
            implicitCast = bin->left->implicitCastTo->toString();
        } else if (bin->right->implicitCastTo) {
            implicitCast = bin->right->implicitCastTo->toString();
        } else {
            auto leftType = std::dynamic_pointer_cast<BinaryOpNode>(bin->left);
            implicitCast = leftType ? leftType->inferredType->toString() : bin->left->inferredType->toString();
        }
        if (implicitCast == "float")
        {
            bin->op = getOperation(bin->op, "float");
            bin->inferredType = std::make_shared<SimpleTypeNode>("float");
        }
        else
        {
            bin->inferredType = std::make_shared<SimpleTypeNode>(implicitCast);
        }
    }   
    if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
        castAllNumbersToType(unary->operand, targetType);
        if (unary->operand->implicitCastTo) {
            unary->implicitCastTo = unary->operand->implicitCastTo;
        } else {
            auto leftType = std::dynamic_pointer_cast<BinaryOpNode>(unary->operand);
            unary->implicitCastTo = leftType ? leftType->inferredType : unary->operand->inferredType;
        }
    }
}

static bool checkIntLimits(const std::string& type, int value) {
    if (type == "i1") return value == 0 || value == 1;
    if (type == "i8") return value >= -128 && value <= 127;
    if (type == "i16") return value >= -32768 && value <= 32767;
    if (type == "i32") return value >= -2147483648 && value <= 2147483647;
    if (type == "i64") return true;
    return true;
}

// Второй проход: кастим и валидируем (для не-auto)
void TypeSymbolVisitor::castAndValidate(const std::shared_ptr<ASTNode>& node, const std::string& targetType, TypeSymbolVisitor* visitor) {
    if (!node) return;
    if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
        int value = num->value;
        std::string fromType = num->inferredType ? num->inferredType->toString() : (num->type ? num->type->toString() : "");
        if (targetType != "float" && !checkIntLimits(targetType, value)) {
            visitor->LogError("Value " + std::to_string(value) + " does not fit in type " + targetType);
        }
        if (fromType != targetType) {
            num->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
        }
        return;
    }
    if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node))
    {
        if (!ident->inferredType)
            visitor->LogError("Expression type is null for variable: " + ident->name);
        else if (ident->inferredType->toString() != targetType)
            if (getTypeRank(ident->inferredType->toString()) > 0)
                ident->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        castAndValidate(bin->left, targetType, visitor);
        castAndValidate(bin->right, targetType, visitor);
        bin->inferredType = std::make_shared<SimpleTypeNode>(targetType);
    }
    if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
        castAndValidate(unary->operand, targetType, visitor);
        unary->inferredType = std::make_shared<SimpleTypeNode>(targetType);
    }
}

// Первый проход: ищем максимальный rank
static void findMaxRank(const std::shared_ptr<ASTNode>& node, int& maxRank) {
    if (!node) return;
    if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
        if (num->inferredType)
            maxRank = std::max(maxRank, getTypeRank(num->inferredType->toString()));
        else if (num->type)
            maxRank = std::max(maxRank, getTypeRank(num->type->toString()));
        return;
    }
    if (auto floatNum = std::dynamic_pointer_cast<FloatNumberNode>(node)) {
        maxRank = std::max(maxRank, getTypeRank("float"));
        return;
    }
    if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        maxRank = std::max(maxRank, getTypeRank(ident->inferredType->toString()));
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        findMaxRank(bin->left, maxRank);
        findMaxRank(bin->right, maxRank);
    }
    if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
        findMaxRank(unary->operand, maxRank);
    }
    if (auto call = std::dynamic_pointer_cast<CallNode>(node)) {
        for (const auto& arg : call->arguments) {
            findMaxRank(arg, maxRank);
        }
    }
}


void TypeSymbolVisitor::castNumbersInBinaryTree(std::shared_ptr<ASTNode> node, const std::string& expectedType) {
    if (!node) return;

    int maxRank = 0;
    std::string targetType = expectedType;

    if (expectedType == "auto") {
        findMaxRank(node, maxRank);
        targetType = getTypeByRank(maxRank);
        if (targetType.empty()) {
            LogError("Cannot deduce type for auto");
            return;
        }

        castAllNumbersToType(node, targetType); 
    } else {
        // Мы не проверим i1 в другом месте
        if (expectedType == "i1")
        {
            findMaxRank(node, maxRank);
            targetType = getTypeByRank(maxRank);
            castAllNumbersToType(node, targetType);
            return;
        }
        castAndValidate(node, targetType, this);
    }
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
        if (type == "i16") { if (maxRank < 3) maxRank = 3; return 3; }
        if (type == "i32") { if (maxRank < 4) maxRank = 4; return 4; }
        if (type == "i64") { if (maxRank < 5) maxRank = 5; return 5; }
        if (type == "float") { if (maxRank < 6) maxRank = 6; return 6; }
        return 0; // unknown type
    };

    if (genericType->baseName == "array") {
        for (auto& statement : block->statements) {
            if (!statement) continue;
            // Для вложенных коллекций рекурсивно
            if (auto subBlock = std::dynamic_pointer_cast<BlockNode>(statement)) {
                validateCollectionElements(genericType->typeParameters[0], subBlock, isAuto);
            } else if (auto binary = std::dynamic_pointer_cast<BinaryOpNode>(statement)) {
                // TODO : Проверить на корректность в пиздец тяжёлых случаях
                binary->left->accept(*this);
                binary->right->accept(*this);
                auto leftType = binary->left->inferredType;
                auto rightType = binary->right->inferredType;
                auto expectedElemType = genericType->typeParameters[0];
                if (auto expectedGeneric = std::dynamic_pointer_cast<GenericTypeNode>(expectedElemType)) {
                    // Вложенная коллекция
                    validateCollectionElements(expectedElemType, statement, isAuto);
                } else {
                    castNumbersInBinaryTree(binary, "auto");
                }
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
        case 3: targetType = "i16"; break;
        case 4: targetType = "i32"; break;
        case 5: targetType = "i64"; break;
        case 6: targetType = "float"; break;
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
                        simple->toString() == "i16" ||
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
        if (generic == "i16") return value >= -32768 && value <= 32767;
        if (generic == "i32") return value >= -2147483648 && value <= 2147483647;
        if (generic == "i64") return true;
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