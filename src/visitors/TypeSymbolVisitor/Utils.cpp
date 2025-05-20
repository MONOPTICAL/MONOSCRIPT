#include "../headers/TypeSymbolVisitor.h"
#include "../../includes/ASTDebugger.hpp"
void TypeSymbolVisitor::LogError(const std::string &message, std::shared_ptr<ASTNode> node)
{
    if (node) {
        int column;

        if (!node->column) column = 0;
        else column = node->column;
        
        ErrorEngine::getInstance().report(
            node->line, 
            column, 
            this->currentModuleName, 
            message
        );
    }
    else
        throw std::runtime_error("[" + this->currentModuleName + "]Semantic Error: " + message);
}

std::shared_ptr<TypeNode> TypeSymbolVisitor::checkForIdentifier(std::shared_ptr<ASTNode>& node)
{
    if (!node) {
        LogError("Node is null", node);
    }
    
    if (auto idNode = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        auto it = contexts.back().variables.find(idNode->name);
        if (it == contexts.back().variables.end()) 
            LogError("Variable not found: " + idNode->name, node);

        auto varAssign = std::dynamic_pointer_cast<VariableAssignNode>(it->second);
        if (!varAssign) 
            LogError("!!!This doesn't need to happen!!!", node);

        if (varAssign->expression == nullptr) {
            if (!varAssign->inferredType)
                LogError("Variable '" + idNode->name + "' has no inferred type", node);
            return varAssign->inferredType;
        }

        if (!varAssign->expression->inferredType)
            LogError("Expression type is null for variable: " + idNode->name, node);

        if (varAssign->expression && varAssign->expression->implicitCastTo) 
            std::cout << varAssign->implicitCastTo->toString() << std::endl;
        return varAssign->expression->inferredType;
    }
    else if (auto varAssignNode = std::dynamic_pointer_cast<VariableAssignNode>(node)) {
        if (varAssignNode->expression == nullptr) {
            if (!varAssignNode->inferredType)
                LogError("Variable '" + varAssignNode->name + "' has no inferred type", node);
            if (varAssignNode->expression && varAssignNode->expression->implicitCastTo)
                std::cout << varAssignNode->implicitCastTo->toString() << std::endl;
            return varAssignNode->inferredType;
        }

        if (!varAssignNode->expression->inferredType)
            LogError("Expression type is null for variable: " + varAssignNode->name, node);

        return varAssignNode->expression->inferredType;
    }

    if (!node->inferredType)
        LogError("Node type is null", node);

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

bool TypeSymbolVisitor::checkLabels(const std::string &label)
{
    static const std::vector<std::string> skipNames = {"if", "else", "for", "while"};
    for (auto it = contexts.rbegin(); it != contexts.rend(); ++it) {
        const auto& context = *it;
        if (!context.currentFunctionName.empty() &&
            std::find(skipNames.begin(), skipNames.end(), context.currentFunctionName) == skipNames.end())
        {
            return std::find(context.labels.begin(), context.labels.end(), label) != context.labels.end();
        }
    }
    return false;
}

int TypeSymbolVisitor::getTypeRank(const std::string& type) {
    if (type == "i1") return 1;
    if (type == "i8") return 2;
    if (type == "i16") return 3;
    if (type == "i32") return 4;
    if (type == "i64") return 5;
    if (type == "float") return 6;
    return 0;
}

std::string TypeSymbolVisitor::getTypeByRank(int rank) {
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
void TypeSymbolVisitor::castAllNumbersToType(const std::shared_ptr<ASTNode>& node, const std::string& targetType) {
    if (!node) return;
    if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
        std::string fromType = num->inferredType ? num->inferredType->toString() : (num->type ? num->type->toString() : "");
        if (fromType != targetType) {
            num->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
            //IC(fromType, targetType, num->implicitCastTo->toString());
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
            visitor->LogError("Value " + std::to_string(value) + " does not fit in type " + targetType, node);
        }
        if (fromType != targetType) {
            num->implicitCastTo = std::make_shared<SimpleTypeNode>(targetType);
        }
        return;
    }
    if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node))
    {
        if (!ident->inferredType)
            visitor->LogError("Expression type is null for variable: " + ident->name, node);
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
void TypeSymbolVisitor::findMaxRank(const std::shared_ptr<ASTNode>& node, int& maxRank) {
    if (!node) return;
    if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
        if (num->implicitCastTo)
            maxRank = std::max(maxRank, getTypeRank(num->implicitCastTo->toString()));
        else if (num->inferredType)
            maxRank = std::max(maxRank, getTypeRank(num->inferredType->toString()));
        else if (num->type)
            maxRank = std::max(maxRank, getTypeRank(num->type->toString()));
        return;
    }
    if (auto floatNum = std::dynamic_pointer_cast<FloatNumberNode>(node)) {
        if (floatNum->implicitCastTo)
            maxRank = std::max(maxRank, getTypeRank(floatNum->implicitCastTo->toString()));
        else if (floatNum->inferredType)
            maxRank = std::max(maxRank, getTypeRank(floatNum->inferredType->toString()));
        return;
    }
    if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        if (ident->implicitCastTo)
            maxRank = std::max(maxRank, getTypeRank(ident->implicitCastTo->toString()));
        else if (ident->inferredType)
            maxRank = std::max(maxRank, getTypeRank(ident->inferredType->toString()));
        return;
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        findMaxRank(bin->left, maxRank);
        findMaxRank(bin->right, maxRank);
    }
    if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
        findMaxRank(unary->operand, maxRank);
    }
    if (auto call = std::dynamic_pointer_cast<CallNode>(node)) {
        if (!checkLabels("@strict"))
        {
            for (const auto& arg : call->arguments) 
                findMaxRank(arg, maxRank);
        }
        else
        {            
            if (contexts.back().functions.find(call->callee) == contexts.back().functions.end()) {
                if (registry.findFunction(call->callee) == nullptr) 
                    LogError("Function not found: " + call->callee, node);
                else
                {
                    auto func = std::dynamic_pointer_cast<FunctionNode>(registry.findFunction(call->callee));
                    maxRank = std::max(maxRank, getTypeRank(func->returnType->toString()));
                }
            }
            else
            {
                auto func = std::dynamic_pointer_cast<FunctionNode>(contexts.back().functions[call->callee]);
                maxRank = std::max(maxRank, getTypeRank(func->returnType->toString()));
            }
        }
    }
}


void TypeSymbolVisitor::castNumbersInBinaryTree(std::shared_ptr<ASTNode>& node, const std::string& expectedType) {
    if (!node) return;

    int maxRank = 0;
    std::string targetType = expectedType;

    if (expectedType == "auto") {
        findMaxRank(node, maxRank);
        targetType = getTypeByRank(maxRank);
        if (targetType.empty()) {
            LogError("Cannot deduce type for auto", node);
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

    if (auto callNode = std::dynamic_pointer_cast<CallNode>(expr)) {
        callNode->accept(*this); // Убедимся, что функция проанализирована и типизирована
        
        if (!callNode->inferredType) {
            LogError("Cannot infer type for function call", callNode);
            return;
        }
        
        // Если функция возвращает массив и ожидается массив массивов - это валидно
        auto funcReturnType = callNode->inferredType;
        if (funcReturnType->toString() == genericType->toString()) 
            return; // Функция прошла проверку
        else
            LogError("Function return type does not match expected type", callNode);
        
    }    
    auto block = std::dynamic_pointer_cast<BlockNode>(expr);
    if (!genericType || !block) 
    {
        if (std::dynamic_pointer_cast<NoneNode>(expr))
            return;

        LogError("Invalid collection initialization", expr);
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
                    castNumbersInBinaryTree(statement, "auto");
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
                    LogError("Element type mismatch: expected " + expectedElemType->toString() + ", got " + elemType->toString(), statement);
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
                        LogError("Key type mismatch: expected " + expectedKeyType->toString() + ", got " + keyType->toString(), keyValue->key);
                }

                // Проверка значения
                if (auto expectedGenericValue = std::dynamic_pointer_cast<GenericTypeNode>(expectedValueType)) {
                    validateCollectionElements(expectedValueType, keyValue->value, isAuto);
                } else if (valueType->toString() != expectedValueType->toString()) {
                    if (!(numericRank(valueType->toString(), maxRank) > 0 && numericRank(expectedValueType->toString(), maxRank) > 0))   
                        LogError("Value type mismatch: expected " + expectedValueType->toString() + ", got " + valueType->toString(), keyValue->value);
                }

                // Проверка ключей на дубликацию
                if (std::find(keys.begin(), keys.end(), keyValue->keyName) != keys.end()) {
                    LogError("Duplicate key: " + keyValue->key->inferredType->toString(), keyValue->key);
                }
                keys.push_back(keyValue->keyName);
            } else {
                statement->accept(*this);
                LogError("Map initialization expects key-value pairs", statement);
            }
        }
    } else {
        LogError("Unknown generic collection: " + genericType->baseName, expr);
    }
    
    if (maxRank > 0) {
        applyImplicitCastToNumeric(expectedType, block, maxRank, isAuto);
    }
}

// Да я захуярил здесь чатгпт я заебался 10 часов над кодом сидеть
void TypeSymbolVisitor::applyImplicitCastToNumeric(
    const std::shared_ptr<TypeNode>& currentExpectedType, // Переименовали expectedType для ясности
    const std::shared_ptr<ASTNode>& expr,
    int globalMaxRank, // Максимальный ранг, найденный во всей внешней структуре (для auto)
    bool isAutoContext) // Является ли внешний контекст auto
{
    auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(currentExpectedType);
    auto block = std::dynamic_pointer_cast<BlockNode>(expr);

    std::string currentTargetType;

    if (auto simpleExpected = std::dynamic_pointer_cast<SimpleTypeNode>(currentExpectedType)) {
        // Если ожидаемый тип на этом уровне - простой (например, мы внутри array<i1>)
        currentTargetType = simpleExpected->toString();
        if (getTypeRank(currentTargetType) == 0 && currentTargetType != "string") { // Не числовой и не строка
             // Логируем или обрабатываем как ошибку, если пытаемся применить числовое приведение к нечисловому типу
             // LogError("Cannot apply numeric cast to non-numeric expected type: " + currentTargetType, expr);
             return;
        }
    } else if (genericType) {
        // Для GenericTypeNode мы не определяем единый currentTargetType здесь,
        // а передаем управление рекурсивным вызовам для его параметров.
    } else {
        // Неизвестный или нерелевантный тип для приведения
        return;
    }


    if (genericType) { // Если текущий ожидаемый тип - это коллекция
        if (genericType->baseName == "array") {
            if (genericType->typeParameters.empty()) return;
            std::shared_ptr<TypeNode> expectedElemType = genericType->typeParameters[0];
            for (auto& statement : block->statements) {
                if (!statement) continue;
                // Рекурсивный вызов для элементов массива.
                // globalMaxRank и isAutoContext передаются дальше, но currentExpectedType меняется.
                applyImplicitCastToNumeric(expectedElemType, statement, globalMaxRank, isAutoContext);
            }
        } else if (genericType->baseName == "map") {
            if (genericType->typeParameters.size() < 2) return;
            std::shared_ptr<TypeNode> expectedKeyType = genericType->typeParameters[0];
            std::shared_ptr<TypeNode> expectedValueType = genericType->typeParameters[1];

            for (auto& statement : block->statements) {
                if (!statement) continue;
                if (auto keyValue = std::dynamic_pointer_cast<KeyValueNode>(statement)) {
                    applyImplicitCastToNumeric(expectedKeyType, keyValue->key, globalMaxRank, isAutoContext);
                    applyImplicitCastToNumeric(expectedValueType, keyValue->value, globalMaxRank, isAutoContext);
                }
            }
        }
    } else if (auto simpleExpectedTypeNode = std::dynamic_pointer_cast<SimpleTypeNode>(currentExpectedType)) {
        // Если currentExpectedType - это простой тип (например, i1, i16, string),
        // и expr - это соответствующий узел (NumberNode, StringNode и т.д.)
        currentTargetType = simpleExpectedTypeNode->toString();
        int expectedRank = getTypeRank(currentTargetType);

        if (expectedRank > 0) { // Если ожидается числовой тип
            if (auto numNode = std::dynamic_pointer_cast<NumberNode>(expr)) {
                std::string actualType = numNode->inferredType ? numNode->inferredType->toString() : "";
                int actualRank = getTypeRank(actualType);

                if (actualRank > 0) { // Если фактический тип тоже числовой
                    std::string finalTargetType = currentTargetType;
                    if (isAutoContext) {
                        // В auto-контексте, если globalMaxRank предлагает более широкий тип, используем его,
                        // НО только если он не "меньше" чем currentTargetType (чтобы не сужать i16 до i1, если currentTargetType был i1)
                        // Это правило нужно уточнить. Пока что, если isAuto, будем доверять типу, выведенному get_common_type_from_list.
                        // `currentExpectedType` уже должен быть результатом get_common_type_from_list.
                        // Поэтому, если isAuto, `currentTargetType` уже является "продвинутым" типом.
                    }
                    
                    // Проверка лимитов для не-auto (или если currentTargetType не изменился от auto-продвижения)
                    if (!isAutoContext || finalTargetType == simpleExpectedTypeNode->toString()) {
                         if (!checkIntLimits(finalTargetType, numNode->value)) {
                            LogError("Value " + std::to_string(numNode->value) + " does not fit in type " + finalTargetType, expr);
                        }
                    }

                    if (actualType != finalTargetType) {
                        numNode->implicitCastTo = std::make_shared<SimpleTypeNode>(finalTargetType);
                    }
                } else if (!actualType.empty()) { // Фактический тип не числовой, а ожидается числовой
                     LogError("Type mismatch: expected numeric type " + currentTargetType + " but got " + actualType, expr);
                }
            } else if (auto floatNode = std::dynamic_pointer_cast<FloatNumberNode>(expr)){
                 // Аналогично для float
                 std::string actualType = floatNode->inferredType ? floatNode->inferredType->toString() : "";
                 if (currentTargetType != "float" && actualType == "float"){
                     LogError("Type mismatch: expected integer type " + currentTargetType + " but got float", expr);
                 } else if (currentTargetType == "float" && actualType != "float" && getTypeRank(actualType) > 0){
                     // Приведение целого к float
                     floatNode->implicitCastTo = std::make_shared<SimpleTypeNode>("float");
                 } // else если оба float или оба не float - ничего не делаем или ошибка
            }
            // Добавить обработку других узлов, если они могут быть числовыми (например, IdentifierNode, CallNode)
        } else if (currentTargetType == "string") {
            // Обработка для строк, если нужно
        }
    }
}

std::shared_ptr<TypeNode> TypeSymbolVisitor::get_common_type_from_list(
    std::vector<std::shared_ptr<TypeNode>>& types,
    std::shared_ptr<ASTNode> error_context_node, // Для LogError в правильном контексте
    bool allow_numeric_promotion_for_simple_types) {

    // DEBUGGING:
     std::cout << "get_common_type_from_list called with types: ";
     for (const auto& t : types) { std::cout << (t ? t->toString() : "null") << ", "; }
     std::cout << "allow_promo: " << allow_numeric_promotion_for_simple_types << std::endl;

    if (types.empty()) {
        // Если список типов пуст, возвращаем "auto", что может быть уточнено позже.
        return std::make_shared<SimpleTypeNode>("auto_empty_list");
    }

    // Фильтруем null типы, чтобы избежать сбоев, но логируем их.
    std::vector<std::shared_ptr<TypeNode>> valid_types;
    for(const auto& t : types) {
        if (t) {
            valid_types.push_back(t);
        } else {
            LogError("Null type encountered in type list, ignoring for common type determination.", error_context_node);
        }
    }

    if (valid_types.empty()) {
        // Если все типы в исходном списке были null.
        LogError("All types in list were null or list was initially empty.", error_context_node);
        return std::make_shared<SimpleTypeNode>("error_all_null_types_in_list");
    }

    // 1. Попытка продвижения простых числовых типов (если разрешено)
    if (allow_numeric_promotion_for_simple_types) {
        bool all_are_simple_numeric = true;
        int max_rank = 0;
        for (const auto& t : valid_types) {
            auto simple_type = std::dynamic_pointer_cast<SimpleTypeNode>(t);
            if (simple_type) {
                int rank = getTypeRank(simple_type->toString());
                if (rank > 0) {
                    max_rank = std::max(max_rank, rank);
                } else { // Простой тип, но не числовой (например, string)
                    all_are_simple_numeric = false;
                    break;
                }
            } else { // Не простой тип (например, GenericTypeNode - вложенная коллекция)
                all_are_simple_numeric = false;
                break;
            }
        }
        if (all_are_simple_numeric && max_rank > 0) {
            return std::make_shared<SimpleTypeNode>(getTypeByRank(max_rank));
        }
    }

    // 2. Попытка продвижения для "массив массивов чисел": array<array<numeric>>
    // Пример: [[1i8], [2i16]] должен стать array<array<i16>>
    bool all_are_array_of_numeric = !valid_types.empty();
    int max_inner_numeric_rank = 0;
    if (!valid_types.empty()) { // Проверяем только если есть хотя бы один валидный тип
        for (const auto& t : valid_types) {
            auto gen_type = std::dynamic_pointer_cast<GenericTypeNode>(t);
            if (gen_type && gen_type->baseName == "array" && gen_type->typeParameters.size() == 1) {
                auto inner_param_type = std::dynamic_pointer_cast<SimpleTypeNode>(gen_type->typeParameters[0]);
                if (inner_param_type) {
                    int rank = getTypeRank(inner_param_type->toString());
                    if (rank > 0) {
                        max_inner_numeric_rank = std::max(max_inner_numeric_rank, rank);
                    } else { // array<нечисловой_простой_тип>
                        all_are_array_of_numeric = false;
                        break;
                    }
                } else { // array<не_простой_тип_параметра> (например, array<array<...>>)
                    all_are_array_of_numeric = false;
                    break;
                }
            } else { // Не GenericTypeNode, не "array", или неверное число параметров
                all_are_array_of_numeric = false;
                break;
            }
        }
    } else {
        all_are_array_of_numeric = false;
    }


    if (all_are_array_of_numeric && max_inner_numeric_rank > 0) {
        auto common_inner_type = std::make_shared<SimpleTypeNode>(getTypeByRank(max_inner_numeric_rank));
        auto common_array_type = std::make_shared<GenericTypeNode>("array");
        common_array_type->typeParameters.push_back(common_inner_type);
        return common_array_type;
    }

    // 3. Если продвижение не применимо, проверяем на строгое соответствие первому валидному типу.
    std::shared_ptr<TypeNode> base_type = valid_types[0];

    // Попытка найти общий тип для map<K1,V1> и map<K2,V2> -> map<commonK, commonV>
    // или array<T1> и array<T2> -> array<commonT>
    bool all_same_generic_base = true;
    auto first_gen_type = std::dynamic_pointer_cast<GenericTypeNode>(base_type);

    if (first_gen_type) {
        for (size_t i = 1; i < valid_types.size(); ++i) {
            auto current_gen_type = std::dynamic_pointer_cast<GenericTypeNode>(valid_types[i]);
            if (!current_gen_type || current_gen_type->baseName != first_gen_type->baseName || current_gen_type->typeParameters.size() != first_gen_type->typeParameters.size()) {
                all_same_generic_base = false;
                break;
            }
        }

        if (all_same_generic_base) {
            if (first_gen_type->baseName == "map" && first_gen_type->typeParameters.size() == 2) {
                std::vector<std::shared_ptr<TypeNode>> all_key_types;
                std::vector<std::shared_ptr<TypeNode>> all_value_types;
                for (const auto& t : valid_types) {
                    auto map_t = std::dynamic_pointer_cast<GenericTypeNode>(t);
                    all_key_types.push_back(map_t->typeParameters[0]);
                    all_value_types.push_back(map_t->typeParameters[1]);
                }
                // Для ключей и значений вызываем get_common_type_from_list.
                // allow_numeric_promotion_for_simple_types должно быть true, чтобы i1 и i8 могли стать i8, например.
                std::shared_ptr<TypeNode> common_k = get_common_type_from_list(all_key_types, error_context_node, true); // Передаем true
                std::shared_ptr<TypeNode> common_v = get_common_type_from_list(all_value_types, error_context_node, true); // Передаем true
                
                // DEBUGGING:
                 std::cout << "get_common_type_from_list (map) common_k: " << (common_k ? common_k->toString() : "null") 
                           << ", common_v: " << (common_v ? common_v->toString() : "null") << std::endl;

                if (common_k && common_v && common_k->toString().rfind("error_", 0) != 0 && common_v->toString().rfind("error_", 0) != 0 &&
                    common_k->toString() != "auto_empty_list" && common_v->toString() != "auto_empty_list") {
                    auto result_map_type = std::make_shared<GenericTypeNode>("map");
                    result_map_type->typeParameters.push_back(common_k);
                    result_map_type->typeParameters.push_back(common_v);
                    return result_map_type;
                } else {
                    // Не удалось найти общий тип для ключей/значений, возвращаемся к строгой проверке
                    // Это место, где может возникнуть проблема, если common_v не выводится как i1
                    // LogError("Debug: Failed to find common K/V for map. K: " + (common_k ? common_k->toString() : "null") + ", V: " + (common_v ? common_v->toString() : "null"), error_context_node);
                }
            } else if (first_gen_type->baseName == "array" && first_gen_type->typeParameters.size() == 1) {
                std::vector<std::shared_ptr<TypeNode>> all_element_types;
                for (const auto& t : valid_types) {
                    auto arr_t = std::dynamic_pointer_cast<GenericTypeNode>(t);
                    all_element_types.push_back(arr_t->typeParameters[0]);
                }
                std::shared_ptr<TypeNode> common_el = get_common_type_from_list(all_element_types, error_context_node, true); // Передаем true

                if (common_el && common_el->toString().rfind("error_", 0) != 0 && common_el->toString() != "auto_empty_list") {
                    auto result_array_type = std::make_shared<GenericTypeNode>("array");
                    result_array_type->typeParameters.push_back(common_el);
                    return result_array_type;
                } else {
                     // Не удалось найти общий тип для элементов, возвращаемся к строгой проверке
                }
            }
        }
    }

    // Если не удалось найти общий GenericType или типы не Generic, используем строгую проверку по первому типу
    for (size_t i = 1; i < valid_types.size(); ++i) {
        if (valid_types[i]->toString() != base_type->toString()) {
            LogError("Inconsistent types in list: expected '" + base_type->toString() + "' but found '" + valid_types[i]->toString() + "'.", error_context_node);
            return std::make_shared<SimpleTypeNode>("error_inconsistent_list_elements");
        }
    }
    return base_type; // Все валидные типы соответствуют первому.
}

std::shared_ptr<TypeNode> TypeSymbolVisitor::infer_collection_type_revised(std::shared_ptr<BlockNode> block) {
    if (!block) {
        LogError("Cannot infer type for a null block node.");
        return std::make_shared<SimpleTypeNode>("error_null_block");
    }

    if (block->statements.empty()) {
        // Для пустого инициализатора `[]` или `{}` тип неоднозначен без контекста.
        // `auto_empty_collection` может быть позже преобразован в `array<auto>` или `map<auto,auto>`.
        return std::make_shared<SimpleTypeNode>("auto_empty_collection");
    }

    // Определяем, map это или array, по первому элементу.
    bool is_map = std::dynamic_pointer_cast<KeyValueNode>(block->statements[0]) != nullptr;

    if (is_map) {
        std::vector<std::shared_ptr<TypeNode>> key_types;
        std::vector<std::shared_ptr<TypeNode>> value_types;
        // Сохраняем узлы ключей для возможного последующего приведения типов
        std::vector<std::shared_ptr<ASTNode>> key_nodes_for_casting;

        for (auto& stmt : block->statements) {
            auto kv_pair = std::dynamic_pointer_cast<KeyValueNode>(stmt);
            if (!kv_pair) {
                LogError("Expected key-value pair in map initializer.", stmt);
                key_types.push_back(std::make_shared<SimpleTypeNode>("error_invalid_map_element"));
                value_types.push_back(std::make_shared<SimpleTypeNode>("error_invalid_map_element"));
                continue;
            }

            // Вывод типа ключа
            kv_pair->key->accept(*this); // Убеждаемся, что inferredType ключа заполнен
            if (!kv_pair->key->inferredType) {
                LogError("Failed to infer type for map key.", kv_pair->key);
                key_types.push_back(std::make_shared<SimpleTypeNode>("error_map_key_type_inference"));
            } else {
                key_types.push_back(kv_pair->key->inferredType);
                key_nodes_for_casting.push_back(kv_pair->key);
            }

            // Вывод типа значения (рекурсивно, если значение - это блок)
            if (auto value_as_block = std::dynamic_pointer_cast<BlockNode>(kv_pair->value)) {
                value_types.push_back(infer_collection_type_revised(value_as_block));
            } else {
                kv_pair->value->accept(*this); // Убеждаемся, что inferredType значения заполнен
                if (!kv_pair->value->inferredType) {
                    LogError("Failed to infer type for map value.", kv_pair->value);
                    value_types.push_back(std::make_shared<SimpleTypeNode>("error_map_value_type_inference"));
                } else {
                    value_types.push_back(kv_pair->value->inferredType);
                }
            }
        }
        
        if (key_types.empty() && value_types.empty() && !block->statements.empty()){
            LogError("Map initializer contains no valid key-value pairs after type inference attempts.", block);
            auto errorMapType = std::make_shared<GenericTypeNode>("map");
            errorMapType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("error_map_structure"));
            errorMapType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("error_map_structure"));
            return errorMapType;
       }

        std::shared_ptr<TypeNode> common_key_type = get_common_type_from_list(key_types, block, true);

        std::shared_ptr<TypeNode> common_value_type = get_common_type_from_list(value_types, block, true);

        // Применяем приведение типов для числовых ключей, если был найден общий числовой тип.
        if (common_key_type && getTypeRank(common_key_type->toString()) > 0) {
            for (auto& key_node_to_cast : key_nodes_for_casting) {
                if (key_node_to_cast->inferredType && key_node_to_cast->inferredType->toString() != common_key_type->toString()) {
                     castNumbersInBinaryTree(key_node_to_cast, common_key_type->toString());
                }
            }
        }
        // Аналогично для значений, если они простые числовые и были повышены.
        if (common_value_type && getTypeRank(common_value_type->toString()) > 0) {
            for (auto& stmt : block->statements) {
                 auto kv_pair = std::dynamic_pointer_cast<KeyValueNode>(stmt);
                 if (!kv_pair || !kv_pair->value || std::dynamic_pointer_cast<BlockNode>(kv_pair->value)) continue; // Пропускаем невалидные или блочные значения

                 if (kv_pair->value->inferredType && getTypeRank(kv_pair->value->inferredType->toString()) > 0 &&
                     kv_pair->value->inferredType->toString() != common_value_type->toString()) {
                    castNumbersInBinaryTree(kv_pair->value, common_value_type->toString());
                 }
            }
        }

        auto resultMapType = std::make_shared<GenericTypeNode>("map");
        resultMapType->typeParameters.push_back(common_key_type);
        resultMapType->typeParameters.push_back(common_value_type);
        return resultMapType;

    } else { // Это Array
        std::vector<std::shared_ptr<TypeNode>> element_types;
        // Сохраняем узлы элементов для возможного последующего приведения типов (только для простых элементов)
        std::vector<std::shared_ptr<ASTNode>> simple_element_nodes_for_casting;

        for (auto& stmt : block->statements) {
            if (auto element_as_block = std::dynamic_pointer_cast<BlockNode>(stmt)) {
                // Рекурсивный вызов для вложенных коллекций
                element_types.push_back(infer_collection_type_revised(element_as_block));
            } else {
                stmt->accept(*this); // Убеждаемся, что inferredType элемента заполнен
                if (!stmt->inferredType) {
                    LogError("Failed to infer type for array element.", stmt);
                    element_types.push_back(std::make_shared<SimpleTypeNode>("error_array_element_type_inference"));
                } else {
                    element_types.push_back(stmt->inferredType);
                    simple_element_nodes_for_casting.push_back(stmt);
                }
            }
        }

        if (element_types.empty() && !block->statements.empty()){
             LogError("Array initializer contains no valid elements after type inference attempts.", block);
             auto errorArrayType = std::make_shared<GenericTypeNode>("array");
             errorArrayType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("error_array_structure"));
             return errorArrayType;
        }

        std::shared_ptr<TypeNode> common_element_type = get_common_type_from_list(element_types, block, true);

        // Применяем приведение типов для простых числовых элементов, если был найден общий числовой тип.
        // Это не применяется, если common_element_type сам является коллекцией (например, array<array<i32>>).
        if (common_element_type && std::dynamic_pointer_cast<SimpleTypeNode>(common_element_type) && getTypeRank(common_element_type->toString()) > 0) {
            for (auto& el_node_to_cast : simple_element_nodes_for_casting) {
                // el_node_to_cast здесь - это узел простого элемента (не блока)
                if (el_node_to_cast->inferredType && getTypeRank(el_node_to_cast->inferredType->toString()) > 0 &&
                    el_node_to_cast->inferredType->toString() != common_element_type->toString()) {
                    castNumbersInBinaryTree(el_node_to_cast, common_element_type->toString());
                }
            }
        }
        
        auto resultArrayType = std::make_shared<GenericTypeNode>("array");
        resultArrayType->typeParameters.push_back(common_element_type);
        return resultArrayType;
    }
}