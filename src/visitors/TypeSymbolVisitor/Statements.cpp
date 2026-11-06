#include "../headers/TypeSymbolVisitor.h"
#include "../../includes/ASTDebugger.hpp"
#include <string_view>

void TypeSymbolVisitor::visit(ProgramNode &node)
{
    this->program = std::make_shared<ProgramNode>(node);

    // Сначала собираем объявления, чтобы функцию можно было вызвать выше её определения
    for (const auto& statement : node.body) {
        if (auto function = std::dynamic_pointer_cast<FunctionNode>(statement); function && function->associated.empty())
            declaredFunctions.emplace(function->name, function);
    }

    for (const auto& statement : node.body) {
        statement->accept(*this);
    }
}

void TypeSymbolVisitor::visit(FunctionNode &node)
{
    // Проверяем, существует ли функция в реестре(в текущем контексте или глобальном)
    // Если вызов выше определения уже положил сюда эту же функцию, это не повтор
    auto existingFunction = contexts.back().functions.find(node.name);
    if(existingFunction != contexts.back().functions.end() && existingFunction->second != node.shared_from_this()) {
        if(contexts[0].functions.find(node.name) != contexts[0].functions.end()) {
            LogError("Function already defined in global context: " + node.name);
        }
        LogError("Function already defined: " + node.name);
    }

    if(!node.associated.empty())
    {
        auto st = registry.findStruct(node.associated);
        if(st == nullptr)
            LogError("Struct not found: " + node.associated);

        if(contexts.size() != 1)
            LogError("Function " + node.name + " cannot be defined in a non-global context");

        // Есть ли в структуре метод с таким же именем(Надо потом сделать более подробную проверку)
        const auto structHasMethodNamed = [](const std::shared_ptr<StructNode>& s,
                                             const std::string& methodName) -> bool
        {
            if (!s || !s->body) return false;

            for (const auto& stmt : std::dynamic_pointer_cast<BlockNode>(s->body)->statements)
            {
                auto fn = std::dynamic_pointer_cast<FunctionNode>(stmt);
#if DEBUG
                if(fn)
                {
                    IC(fn->name);
                    IC(fn->labels);
                    IC(methodName);
                }
#endif
                if (fn && fn->name == methodName && fn->labels.end() != std::find(fn->labels.begin(), fn->labels.end(), "@decl"))
                    return true;
                
            }
#if DEBUG
            IC(methodName);
#endif
            return false;
        };

        if(!structHasMethodNamed(st, node.name))
            LogError("Method not declared or already implemented in struct '" + node.associated + "': " + node.associated + "::" + node.name);
        
        node.associated = st->name; // Нормализуем имя структуры
    }

    std::vector<std::string> labels = contexts.back().labels;
    labels.insert(labels.end(), node.labels.begin(), node.labels.end()); // Добавляем метки функции в текущий контекст

    /*
    Пока что не проверяем, что функция является методом структуры
    */
    if(registry.findStruct(contexts.back().currentFunctionName)) {
        node.associated = contexts.back().currentFunctionName;
    }
   
    std::unordered_map<std::string, std::shared_ptr<TypeNode>> args = {};

    // Проходим по параметрам функции
    for (const auto& param : node.parameters) {
        param.first->accept(*this); // Проверяем тип параметра
        // Тут типы параметров не проверяются, так как они могут быть разными и это нормально

        /* TODO: Проверь перекрытие имён
        if(std::find(labels.begin(), labels.end(), "@pure") == labels.end()) { // Если функция не является чистой
            if (contexts.back().variables.find(param.second) != contexts.back().variables.end()) {
                LogError("Parameter already defined: " + param.second);
            }
        }
        */

        if (std::find(node.labels.begin(), node.labels.end(), param.second) != node.labels.end()) {
            LogError("Parameter cannot have the same name as a label: " + param.second);
        }

        if (args.find(param.second) != args.end()) {
            LogError("Parameter already defined: " + param.second, node.shared_from_this());
        }

        // Добавляем параметр в реестр
        args[param.second] = param.first;
    }

    // Если функция не является чистой, то хуярим ей все предыдущие переменные
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> variables = {};
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> functions = {};

    if(std::find(labels.begin(), labels.end(), "@pure") == labels.end())
    {
        variables = contexts.back().variables;
        functions = contexts.back().functions;
    }
    else
    {
        for (const auto& func : contexts.back().functions) {
            if (auto funcNode = std::dynamic_pointer_cast<FunctionNode>(func.second)) {
                if (std::find(funcNode->labels.begin(), funcNode->labels.end(), "@pure") != funcNode->labels.end()) { // Если функция является чистой
                    functions[func.first] = func.second;
                }
            }
        }
    }
    
    if(std::find(labels.begin(), labels.end(), "@entry") != labels.end() || node.name == "main")
        if(node.returnType->toString() != "i32" && node.returnType->toString() != "void")
            LogError("Function " + node.name + " must return i32 or void");

    // Если не вывилась ошибка, добавляем функцию в реестр
    Context currentFunction = {
        .labels = labels, // Добавляем метки функции
        .variables = variables, // Добавляем переменные текущего контекста
        .functions = functions, // Добавляем функции текущего контекста
        .currentFunctionName = node.name,       // Имя функции 
        .returnType = node.returnType,          // Тип возвращаемого значения
        .returnedValue = false                  // Возвращаемое значение
    };

    // Добавляем параметры функции в текущий контекст
    for (const auto& param : args) {
        auto varNode = std::make_shared<VariableAssignNode>(
            param.first, false, std::dynamic_pointer_cast<TypeNode>(param.second), nullptr);
        
        varNode->inferredType = varNode->type; // <-- инициализация inferredType

        // Добавляем переменную в реестр
        currentFunction.variables[param.first] = varNode;
    }

    // Если функция находится в глобальном контексте, то добавляем ее в глобальный реестр(глобальный контекст - это первый элемент в векторе)
    // Если функция не является методом класса, то добавляем ее в глобальный реестр
    if(contexts.size() == 1)
        contexts[0].functions[node.name] = node.shared_from_this();
    else
        contexts.back().functions[node.name] = node.shared_from_this();

    currentFunction.functions[node.name] = node.shared_from_this(); // Добавляем функцию в текущий контекст
    
    const bool noDecl =
        std::find(labels.begin(), labels.end(), "@decl") != labels.end();

    if (noDecl)
    {
        node.inferredType = node.returnType;
#if DEBUG
        IC(contexts.back().currentFunctionName);
        IC();
#endif
        return;
    }

    // Добавляем функцию в реестр
    contexts.push_back(currentFunction);

    // Проверяем тело функции
    node.body->accept(*this);

    if(contexts.back().returnedValue == false && contexts.back().returnType->toString() != "void") {
        LogError("Function " + node.name + " must return a value of type " + contexts.back().returnType->toString());
    }

    contexts.pop_back(); // Убираем текущий контекст
    
    node.inferredType = node.returnType; // Устанавливаем тип функции
}

void TypeSymbolVisitor::visit(BlockNode &node)
{
    for (const auto& statement : node.statements) {
        statement->accept(*this);
    }
}

static std::string getType(std::shared_ptr<ASTNode> expression, std::string type) {
    if (auto num = std::dynamic_pointer_cast<NumberNode>(expression)) 
        return num->inferredType->toString();
    else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(expression)) 
        if (bin->op == "and" || bin->op == "or" || bin->op == "==" || bin->op == "!=" || bin->op.rfind("icmp_", 0) == 0 || bin->op.rfind("fcmp_", 0) == 0)
            return "i1";
        else
            return type;
    else if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(expression))
        return (unary->op == "neg") ? type : "i1";
    else
        return type;
};

void TypeSymbolVisitor::visit(VariableAssignNode &node)
{
    // Проверяем, существует ли переменная в реестре
    if (contexts.back().variables.find(node.name) != contexts.back().variables.end()) {
        LogError("Variable already defined: " + node.name);
    }

    // Проверяем тип переменной
    node.type->accept(*this);
    std::string varType = node.type->toString();
    bool isAuto = false;
    bool isStrict = checkLabels("@strict");

    if (varType == "auto")
        if(!isStrict)
            isAuto = true;
        else
            LogError("auto is not allowed in strict mode", node.shared_from_this());

    if (
        varType == "none"
        || varType == "null"
        || varType == "void"
    ) {
        LogError("Type cannot be " + varType, node.shared_from_this());
    }
    
    if (isAuto) {
        if (std::dynamic_pointer_cast<NoneNode>(node.expression))
            LogError("Cannot infer type of empty value for variable '" + node.name + "', declare the type explicitly", node.shared_from_this());

        if (auto block = std::dynamic_pointer_cast<BlockNode>(node.expression)) {
            std::shared_ptr<TypeNode> inferredType = infer_collection_type_revised(block);

            if (inferredType->toString() == "auto_empty_collection") {
                bool blockIsMapSyntax = false; 

                if (blockIsMapSyntax) {
                    auto mapType = std::make_shared<GenericTypeNode>("map");
                    mapType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("auto"));
                    mapType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("auto"));
                    node.type = mapType;
                } else {
                    auto arrayType = std::make_shared<GenericTypeNode>("array");
                    arrayType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("auto"));
                    node.type = arrayType;
                }
                LogError("Ambiguous empty collection initializer for 'auto' variable '" + node.name + "'. Resolved based on syntax (assuming array if unclear).", node.shared_from_this());
            } else if (inferredType->toString().rfind("error_", 0) == 0) {
                LogError("Failed to infer type for collection assigned to 'auto' variable '" + node.name + "': " + inferredType->toString(), node.expression);
            }
            else {
                node.type = inferredType;
            }
            varType = node.type->toString(); // Обновляем varType после вывода
        }
    }
    if (varType.starts_with("array")
        || varType.starts_with("map")
    ) {
#if DEBUG
        ASTDebugger::debug(node.expression);
#endif
        validateCollectionElements(node.type, node.expression, isAuto);
        
    }
    else
    {
#if DEBUG
        IC(node.expression->inferredType);
#endif
        node.expression->accept(*this);
        auto exprType = node.expression->implicitCastTo ? node.expression->implicitCastTo
                                                        : node.expression->inferredType;

        if (std::dynamic_pointer_cast<GenericTypeNode>(exprType)) {
            if (isAuto)
                node.type = exprType;
            else if (exprType->toString() != varType)
                LogError("Type mismatch: expected " + varType + ", got " + exprType->toString(), node.expression);
        }
        else if (!isAuto) {
            if (exprType->toString() != "none")
                checkConversion(node.expression, varType);
        }
        else {
            if (exprType->toString() != "none" && exprType->toString() != "string")
                castNumbersInBinaryTree(node.expression, "auto");

            if(node.expression->implicitCastTo)
                node.type = std::make_shared<SimpleTypeNode>(getType(node.expression, node.expression->implicitCastTo->toString()));
            else
                node.type = std::make_shared<SimpleTypeNode>(getType(node.expression, node.expression->inferredType->toString()));
        }
    }

    // Добавляем переменную в реестр
    auto varNode = std::make_shared<VariableAssignNode>(node.name, node.isConst, node.type, node.expression);
    varNode->inferredType = node.type; // Устанавливаем тип переменной

    contexts.back().variables[node.name] = varNode;
}

static bool isCompareOperator(const std::string& op) {
    return op.starts_with("icmp_") || op.starts_with("fcmp_") ||
           op == "and" || op == "or";
}

void TypeSymbolVisitor::visit(ReturnNode &node)
{
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("Return statement outside of function", node.shared_from_this());
    }

    // Проверяем тип возвращаемого значения
    if (node.expression)
    {
        if (auto block = std::dynamic_pointer_cast<BlockNode>(node.expression))
        {
            node.expression->accept(*this);
            validateCollectionElements(contexts.back().returnType, node.expression, false);
        }
        else
        {
            node.expression->accept(*this);
            std::string expectedType = contexts.back().returnType->toString();
            const bool bareReturn = node.expression->inferredType && node.expression->inferredType->toString() == "null";
            if (!(bareReturn && expectedType == "void"))
                checkConversion(node.expression, expectedType, "Return: ");
        }
    }

    contexts.back().returnedValue = true; // Устанавливаем, что функция вернула значение
}

// TODO: Переделать функцию полность потому что она хуйня ебанная
void TypeSymbolVisitor::visit(VariableReassignNode& node) {
    // Проверяем, существует ли переменная в реестре
    if (contexts.back().variables.find(node.name) == contexts.back().variables.end()) {
        LogError("Variable not found: " + node.name, node.shared_from_this());
    }

    std::shared_ptr<VariableAssignNode> varAssign = std::dynamic_pointer_cast<VariableAssignNode>(contexts.back().variables[node.name]);
    std::shared_ptr<TypeNode> varType = varAssign->inferredType;
    std::string varTypeStr = varType->toString();

    if (varAssign->isConst) {
        LogError("Cannot reassign constant variable: " + node.name, node.shared_from_this());
    }

    std::string expressionType;
    if (varTypeStr.starts_with("array")
        || varTypeStr.starts_with("map")
    ) {
        node.expression->accept(*this);
        validateCollectionElements(varType, node.expression, false);
        auto Block = std::dynamic_pointer_cast<BlockNode>(node.expression);
        auto Generic = std::dynamic_pointer_cast<GenericTypeNode>(varType);

        // a = [] или a = b: validateCollectionElements уже всё проверил
        if (!Block) {
            varAssign->expression = node.expression;
            return;
        }

        if (auto keyValue = std::dynamic_pointer_cast<KeyValueNode>(Block->statements[0])) {
            std::vector<std::string> types = { keyValue->key->inferredType->toString(), keyValue->value->inferredType->toString() };
            if (types[0] != Generic->typeParameters[0]->toString()) {
                LogError("Key type mismatch: expected " + Generic->typeParameters[0]->toString() + ", got " + types[0], node.expression);
            }
            if (types[1] != Generic->typeParameters[1]->toString()) {
                LogError("Value type mismatch: expected " + Generic->typeParameters[1]->toString() + ", got " + types[1], node.expression);
            }
            expressionType = "map<" + types[0] + ", " + types[1] + ">";
        } 
        else {
            if (Block->statements[0]->inferredType->toString() != Generic->typeParameters[0]->toString()) {
                if (Generic->typeParameters[0]->toString() == "i1")
                    if (auto binaryOp = std::dynamic_pointer_cast<BinaryOpNode>(Block->statements[0])) {
                        if (isCompareOperator(binaryOp->op)) {
                            expressionType = "array<i1>";
                        } else {
                            LogError("Type mismatch: expected i1, got " + Block->statements[0]->inferredType->toString(), node.expression);
                        }
                    }
                else
                {
                    
                    LogError("Element type mismatch: expected " + Generic->typeParameters[0]->toString() + ", got " + Block->statements[0]->inferredType->toString(), node.expression);
                }
            }
            else 
                expressionType = "array<" + Block->statements[0]->inferredType->toString() + ">";
        } 
    }
    else
    {
        node.expression->accept(*this);
        checkConversion(node.expression, varTypeStr);
        expressionType = varTypeStr;
    }

    if (expressionType != varTypeStr) 
            LogError("Type mismatch: expected " + varTypeStr + ", got " + expressionType, node.expression);

    // Добавляем переменную в реестр
    varAssign->expression = node.expression;
}

void TypeSymbolVisitor::visit(IfNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("If statement outside of function", node.shared_from_this());
    }

    node.condition->accept(*this);
    std::string conditionType = node.condition->inferredType->toString();

    if (conditionType != "i1") 
        LogError("If condition must be of type i1, got " + conditionType, node.condition);
    else 
    {
        auto binaryOp = std::dynamic_pointer_cast<BinaryOpNode>(node.condition);
        if(binaryOp && binaryOp->op != "strcmp_eq") 
                castNumbersInBinaryTree(node.condition, "auto");
        else 
                castNumbersInBinaryTree(node.condition, "auto");
    }

    // Создаем новый контекст для блока if
    contexts.push_back(contexts.back());

    contexts.back().currentFunctionName = "if";

    // Проверяем блок if
    node.thenBlock->accept(*this);
    const bool thenReturned = contexts.back().returnedValue;

    // Убираем контекст блока if
    contexts.pop_back();

    // Проверяем блок else
    if (node.elseBlock) {
        // Создаем новый контекст для блока else
        contexts.push_back(contexts.back());

        contexts.back().currentFunctionName = "else";

        // Проверяем блок else
        node.elseBlock->accept(*this);
        const bool elseReturned = contexts.back().returnedValue;

        // Убираем контекст блока else
        contexts.pop_back();

        // Если return есть в обеих ветках, то функция точно вернула значение
        if (thenReturned && elseReturned)
            contexts.back().returnedValue = true;
    }

}

void TypeSymbolVisitor::visit(ForNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("For statement outside of function", node.shared_from_this());
    }

    // Проверяем тип переменной
    node.iterable->accept(*this);
    std::string iterableType = node.iterable->inferredType->toString();

    if (iterableType == "none" || iterableType == "null" || iterableType == "void") {
        LogError("Type cannot be " + iterableType, node.iterable);
    }

    node.varType = node.iterable->inferredType; // Устанавливаем тип итератора 

    // Создаем новый контекст для блока for
    contexts.push_back(contexts.back());

    contexts.back().currentFunctionName = "for";

    // Проверяем блок for
    node.body->accept(*this);

    // Убираем контекст блока for
    contexts.pop_back();
}

void TypeSymbolVisitor::visit(WhileNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("If statement outside of function", node.shared_from_this());
    }

    node.condition->accept(*this);
    std::string conditionType = node.condition->inferredType->toString();

    if (conditionType != "i1") 
        LogError("If condition must be of type i1, got " + conditionType, node.condition);
    else 
    {
        auto binaryOp = std::dynamic_pointer_cast<BinaryOpNode>(node.condition);
        if(binaryOp && binaryOp->op != "strcmp_eq") 
                castNumbersInBinaryTree(node.condition, "auto");
        else 
                castNumbersInBinaryTree(node.condition, "auto");
    }

    // Создаем новый контекст для блока while
    contexts.push_back(contexts.back());

    contexts.back().currentFunctionName = "while";

    // Проверяем блок while
    node.body->accept(*this);

    // Убираем контекст блока while
    contexts.pop_back();
}

void TypeSymbolVisitor::visit(BreakNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("Break statement outside of function", node.shared_from_this());
    }

    bool inLoop = false;
    for (auto it = contexts.rbegin(); it != contexts.rend(); ++it) {
        if (it->currentFunctionName == "for" || it->currentFunctionName == "while") {
            inLoop = true;
            break;
        }
    }

    if (!inLoop) {
        LogError("Break statement outside of loop", node.shared_from_this());
    }

    node.inferredType = std::make_shared<SimpleTypeNode>("void");
}


// TODO: Сделать проверку 
void TypeSymbolVisitor::visit(ContinueNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("Continue statement outside of function", node.shared_from_this());
    }

    bool inLoop = false;
    for (auto it = contexts.rbegin(); it != contexts.rend(); ++it) {
        if (it->currentFunctionName == "for" || it->currentFunctionName == "while") {
            inLoop = true;
            break;
        }
    }

    if (!inLoop) {
        LogError("Continue statement outside of loop", node.shared_from_this());
    }
    node.inferredType = std::make_shared<SimpleTypeNode>("void");
}

void TypeSymbolVisitor::visit(CallNode& node) { 
    // Делаем список типов аргументов
    std::vector<std::shared_ptr<TypeNode>> argTypes;
    for (size_t i = 0; i < node.arguments.size(); ++i) {
        node.arguments[i]->accept(*this);

        if (node.arguments[i]->implicitCastTo)
            argTypes.push_back(node.arguments[i]->implicitCastTo);
        else
            argTypes.push_back(node.arguments[i]->inferredType);
    }

    // Проверяем, существует ли функция в реестре
    if (contexts.back().functions.find(node.callee) == contexts.back().functions.end()) {
        auto declared = declaredFunctions.find(node.callee);

        // Функция объявлена в модуле, в том числе ниже этого вызова
        if (declared != declaredFunctions.end())
            contexts.back().functions[node.callee] = declared->second;

        // Точное совпадение типов аргументов
        else if (registry.findFunction(node.callee, argTypes) != nullptr)
            contexts.back().functions[node.callee] = registry.findFunction(node.callee, argTypes);

        // Поиск функции с совпадением по имени
        else if (registry.findFunction(node.callee) != nullptr) 
            contexts.back().functions[node.callee] = registry.findFunction(node.callee);

        // Функция нихуя не найдена
        else
            LogError("Function not found: " + node.callee, node.shared_from_this());
    }

    // Проверяем типы аргументов
    std::shared_ptr<FunctionNode> func = std::dynamic_pointer_cast<FunctionNode>(contexts.back().functions[node.callee]);

    if (checkLabels("@pure") && std::find(func->labels.begin(), func->labels.end(), "@pure") == func->labels.end())
        LogError("Pure function cannot call impure function: " + node.callee, node.shared_from_this());

    // Проверяем количество аргументов
    if (argTypes.size() != func->parameters.size()) {
        LogError("Function " + node.callee + " expects " + std::to_string(func->parameters.size()) + " arguments, got " + std::to_string(argTypes.size()), node.shared_from_this());
    }

    // Проверяем типы аргументов
    for (size_t i = 0; i < argTypes.size(); ++i) {
        std::string paramType = func->parameters[i].first->toString();
        checkConversion(node.arguments[i], paramType, "Function " + node.callee + ", argument " + std::to_string(i + 1) + ": ");
    }

    node.inferredType = func->returnType; // Устанавливаем тип функции
}
