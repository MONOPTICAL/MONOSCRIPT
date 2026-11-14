#include "../headers/TypeSymbolVisitor.h"
#include <algorithm>
std::shared_ptr<TypeNode> functionTypeOf(const FunctionNode& function); // Types.cpp

// Только литералы: они принимают нужный тип сами, каст им не нужен
bool isLiteralExpression(const std::shared_ptr<ASTNode>& node) {
    if (auto number = std::dynamic_pointer_cast<NumberNode>(node)) return !number->isCharLiteral;
    if (std::dynamic_pointer_cast<FloatNumberNode>(node)) return true;
    if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node)) return isLiteralExpression(unary->operand);
    if (auto binary = std::dynamic_pointer_cast<BinaryOpNode>(node)) return isLiteralExpression(binary->left) && isLiteralExpression(binary->right);
    return false;
}

void TypeSymbolVisitor::visit(BinaryOpNode& node) {
    node.left->accept(*this);
    node.right->accept(*this);

    // Важен тип значения после приведения: (a->i32) + 1 при float a это целая операция
    std::shared_ptr<TypeNode> leftType = node.left->implicitCastTo ? node.left->implicitCastTo : node.left->inferredType;
    std::shared_ptr<TypeNode> rightType = node.right->implicitCastTo ? node.right->implicitCastTo : node.right->inferredType;
    
    // Литерал рядом с float становится float: иначе f == 0.1 сравнивало бы f с более точным 0.1 и было бы ложью
    if (rightType && rightType->toString() == "float" && isLiteralExpression(node.left) && !isLiteralExpression(node.right)) {
        castAllNumbersToType(node.left, "float");
        leftType = rightType;
    } else if (leftType && leftType->toString() == "float" && isLiteralExpression(node.right) && !isLiteralExpression(node.left)) {
        castAllNumbersToType(node.right, "float");
        rightType = leftType;
    }

    // @strict обещает, что неявных приведений нет: разные числовые типы в одной операции требуют каста
    if (checkLabels("@strict") && leftType && rightType && leftType->toString() != rightType->toString()
        && getTypeRank(leftType->toString()) > 0 && getTypeRank(rightType->toString()) > 0
        && !isLiteralExpression(node.left) && !isLiteralExpression(node.right)) {
        LogError("Implicit cast is not allowed in @strict mode: " + leftType->toString() + " and " + rightType->toString() + ", use ->", node.right);
    }

    // Знаковое и беззнаковое смешиваются, только если одно целиком влезает в другое: u8 + i32 можно, i32 + u32 нельзя
    if (leftType && rightType && isIntegerTypeName(leftType->toString()) && isIntegerTypeName(rightType->toString())
        && (isUnsignedTypeName(leftType->toString()) || isUnsignedTypeName(rightType->toString()))
        && !isLiteralExpression(node.left) && !isLiteralExpression(node.right)
        && !isLosslessInt(leftType->toString(), rightType->toString()) && !isLosslessInt(rightType->toString(), leftType->toString())) {
        LogError("Cannot mix " + leftType->toString() + " and " + rightType->toString() + ": one does not fit in the other, use ->", node.right);
    }

    std::shared_ptr<BinaryOpNode> root = std::make_shared<BinaryOpNode>(node.left, node.op, node.right);
    root->implicitCastTo = node.implicitCastTo; // Явный каст (a + b)->i32 не должен потеряться
    root->line = node.line; root->column = node.column;

    // Обработчики операторов создают новый узел, а место в исходном коде остаётся прежним
    auto finish = [&]() {
        // Общий тип без знака: деление, остаток, сравнение, сдвиг вправо и степень берут беззнаковые инструкции.
        // Только для чисел: у склейки строк правая часть ещё не обойдена
        const bool numeric = root->op != "scat" && root->op != "and" && root->op != "or" && !root->op.starts_with("strcmp_")
            && (root->left->implicitCastTo || root->left->inferredType) && (root->right->implicitCastTo || root->right->inferredType);
        const std::string leftOperand = numeric ? resolveExprType(root->left) : "", rightOperand = numeric ? resolveExprType(root->right) : "";
        const std::string common = getTypeRank(leftOperand) >= getTypeRank(rightOperand) ? leftOperand : rightOperand;
        if (isUnsignedTypeName(common)) {
            static const std::map<std::string, std::string> unsignedOps = {
                {"sdiv", "udiv"}, {"srem", "urem"}, {"shr", "lshr"}, {"ipow", "upow"},
                {"icmp_slt", "icmp_ult"}, {"icmp_sgt", "icmp_ugt"}, {"icmp_sle", "icmp_ule"}, {"icmp_sge", "icmp_uge"},
            };
            if (auto op = unsignedOps.find(root->op); op != unsignedOps.end()) root->op = op->second;
            if (!root->op.starts_with("icmp_")) root->inferredType = registry.findType(common);
        }
        const int line = node.line, column = node.column;
        node = *root;
        node.line = line; node.column = column;
    };

    // "+"
    if (node.op == "+" || node.op == "add" || node.op == "fadd") {
        handlePlusOperator(root, leftType, rightType);
        finish();
        return;
    }

    // "-"
    if (node.op == "-" || node.op == "sub" || node.op == "fsub") {
        handleMinusOperator(root, leftType, rightType);
        finish();
        return;
    }

    // "*"
    if (node.op == "*" || node.op == "mul" || node.op == "fmul") {
        handleMulOperator(root, leftType, rightType);
        finish();
        return;
    }

    // "/"
    if (node.op == "/" || node.op == "sdiv" || node.op == "udiv" || node.op == "fdiv") {
        handleDivOperator(root, leftType, rightType);
        finish();
        return;
    }

    // "%"
    if (node.op == "%" || node.op == "srem" || node.op == "urem" || node.op == "frem") {
        handleModOperator(root, leftType, rightType);
        finish();
        return;
    }

    // band, bor, xor, shl, shr
    if (node.op == "band" || node.op == "bor" || node.op == "xor" || node.op == "shl" || node.op == "shr" || node.op == "lshr") {
        handleBitwiseOperator(root, leftType, rightType);
        finish();
        return;
    }

    // "**"
    if (node.op == "**" || node.op == "ipow" || node.op == "upow" || node.op == "fpow") {
        handlePowOperator(root, leftType, rightType);
        finish();
        return;
    }

    auto getCmpOp = [](const std::string& op) -> std::string {
        if (op == "==" || op == "eq") return "eq";
        else if (op == "!=" || op == "ne") return "ne";
        else if (op == "<") return "slt";
        else if (op == ">") return "sgt";
        else if (op == "<=") return "sle";
        else if (op == ">=") return "sge";
        return "";
    };

    // "==", "!=", "<", ">", "<=", ">="
    if (getCmpOp(node.op) != "") {
        handleCompareOperator(root, leftType, rightType);
        finish();
        return;
    }

    // and, or 
    if (node.op == "and" || node.op == "or") {
        handleLogicalOperator(root, leftType, rightType);
        finish();
        return;
    }

    // Синтаксис есть, а работы с ним пока нет
    if (node.op == "|>")
        LogError("Operator '" + node.op + "' is not implemented yet", node.right);
}

void TypeSymbolVisitor::visit(UnaryOpNode& node) {
    node.operand->accept(*this);
    std::shared_ptr<TypeNode> operandType = node.operand->inferredType;

    if (node.op == "try") {
        auto source = std::dynamic_pointer_cast<GenericTypeNode>(operandType);
        if (!source || source->baseName != "result")
            LogError("Postfix '?' requires result<T>", node.shared_from_this());
        auto destination = std::dynamic_pointer_cast<GenericTypeNode>(contexts.back().returnType);
        if (!node.fallback && !node.errorBody && contexts.back().resultHandlerDepth == 0 &&
            (!destination || destination->baseName != "result" || contexts.back().currentFunctionName.empty()))
            LogError("Postfix '?' requires a function returning result<T>", node.shared_from_this());
        node.inferredType = source->typeParameters[0];
        if (node.inferredType->toString() == "void" && node.implicitCastTo)
            LogError("Cannot cast void after postfix '?'", node.shared_from_this());
        if (node.fallback) {
            if (node.inferredType->toString() == "void")
                LogError("result<void> cannot have a fallback value", node.shared_from_this());
            const auto type = node.inferredType->toString();
            if (type.starts_with("array<") || type.starts_with("map<")) {
                validateCollectionElements(node.inferredType, node.fallback, false);
                node.fallback->inferredType = node.inferredType;
            } else {
                node.fallback->accept(*this);
                checkConversion(node.fallback, type, "Fallback: ");
            }
        }
        if (node.errorBody) {
            if (contexts.back().currentFunctionName.empty())
                LogError("Local action handler outside of function", node.shared_from_this());
            if (contexts.back().variables.count("error"))
                LogError("Variable already defined: error, the local handler binds error itself; rename the variable", node.shared_from_this());
            const auto assigned = assignedFinals, maybe = maybeAssignedFinals;
            contexts.push_back(contexts.back());
            contexts.back().returnedValue = false;
            auto errorType = std::make_shared<GenericTypeNode>("result");
            errorType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("void"));
            auto error = std::make_shared<VariableAssignNode>("error", true, errorType, nullptr);
            error->inferredType = errorType;
            contexts.back().variables["error"] = error;
            node.errorBody->accept(*this);
            const auto& last = node.errorBody->statements.back();
            if (!contexts.back().returnedValue && !std::dynamic_pointer_cast<BreakNode>(last) &&
                !std::dynamic_pointer_cast<ContinueNode>(last))
                LogError("Local action handler must end with return, break or continue", node.errorBody);
            contexts.pop_back();
            assignedFinals = assigned; maybeAssignedFinals = maybe;
        }
        checkCharCast(node);
        return;
    }

    // char не число: сначала каст
    if (operandType && operandType->toString() == "char" && !node.operand->implicitCastTo)
        LogError("Unsupported operand type for unary operator: char, use ->i32", node.shared_from_this());

    // Узел уже обходили и переименовали оператор, второй обход ничего не меняет
    if (node.op == "nullcheck")
    {
        node.inferredType = registry.findType("i1");
        return;
    }
    if (node.op == "neg" || node.op == "not")
    {
        node.inferredType = operandType;
        return;
    }

    if (node.op == "bnot")
    {
        // bnot 0->u64: тип это каст операнда, а не тип литерала
        if (node.operand->implicitCastTo) operandType = node.operand->implicitCastTo;
        // Голый литерал это i32, а не i1: bnot 0 это -1, а не логическое не
        else if (auto number = std::dynamic_pointer_cast<NumberNode>(node.operand); number && !number->isBoolLiteral && getTypeRank(operandType->toString()) < getTypeRank("i32")) {
            operandType = registry.findType("i32");
            number->implicitCastTo = operandType;
        }
        const std::string type = operandType ? operandType->toString() : "";
        if (!isIntegerTypeName(type))
            LogError("Unsupported operand type for 'bnot': " + type + ", only integers", node.shared_from_this());
        node.inferredType = operandType;
        return;
    }

    // ?r это r.ok. На остальных типах ?x появится вместе с nullable-типами
    if (node.op == "?")
    {
        auto generic = std::dynamic_pointer_cast<GenericTypeNode>(operandType);
        if (!generic || generic->baseName != "result") {
            LogError("Operator '?' is not implemented yet for " + (operandType ? operandType->toString() : std::string("this value")) + ", only for result<T>", node.shared_from_this());
            return;
        }
        node.op = "nullcheck";
        node.inferredType = registry.findType("i1");
        return;
    }
    else if (node.op == "!")
    {
        if (std::dynamic_pointer_cast<CallNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<BinaryOpNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<AccessExpression>(node.operand)) {}
        else if (std::dynamic_pointer_cast<IdentifierNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<LambdaNode>(node.operand)) {}
        else
        {
            LogError("Unsupported operand type for '!'", node.shared_from_this());
            return;
        }

        node.op = "not";
    }
    else if (node.op == "-")
    {
        if (std::dynamic_pointer_cast<CallNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<BinaryOpNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<AccessExpression>(node.operand)) {}
        else if (std::dynamic_pointer_cast<IdentifierNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<NumberNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<FloatNumberNode>(node.operand)) {}
        else
        {
            LogError("Unsupported operand type for '-'", node.shared_from_this());
            return;
        }

        node.op = "neg";
    }
    else
    {
        LogError("Unsupported unary operator: " + node.op, node.shared_from_this());
        return;
    }

    node.inferredType = operandType;
}

void TypeSymbolVisitor::visit(LambdaNode &node)
{
    if (node.lifted) { node.inferredType = functionTypeOf(*node.lifted); return; } // Уже вынесена

    // Тело-выражение это return этого выражения
    std::shared_ptr<ASTNode> body = node.body;
    if (!std::dynamic_pointer_cast<BlockNode>(body)) {
        std::shared_ptr<ASTNode> statement = body;
        if (node.returnType->toString() != "void") statement = std::make_shared<ReturnNode>(body);
        statement->line = node.line; statement->column = node.column;
        body = std::make_shared<BlockNode>(std::vector<std::shared_ptr<ASTNode>>{statement});
    }

    static int lambdas = 0;
    auto lifted = std::make_shared<FunctionNode>("__lambda" + std::to_string(++lambdas), "", node.returnType, node.parameters, std::vector<std::string>{}, body);
    lifted->line = node.line; lifted->column = node.column;
    lifted->isLambda = true;

    // Захват по значению в момент создания, как у Java: внутри лямбды захваченное только читается.
    // shortcut: захватываются все видимые локальные переменные, а не только использованные. Считать использованные, если замыкания станут тяжёлыми
    if (contexts.size() > 1)
        for (const auto& [name, variable] : contexts.back().variables) {
            auto global = contexts[0].variables.find(name);
            if (global != contexts[0].variables.end() && global->second == variable) continue;
            if (std::any_of(node.parameters.begin(), node.parameters.end(), [&](const auto& p) { return p.second == name; })) continue;
            auto type = variable->inferredType;
            if (auto declared = std::dynamic_pointer_cast<VariableAssignNode>(variable); declared && !type) type = declared->type;
            if (!type || type->toString() == "auto") continue;
            lifted->captures.push_back({type, name});
        }
    node.lifted = lifted;
    declaredFunctions[lifted->name] = lifted;
    genericInstances.push_back(lifted); // Кодгену нужна как обычная функция верхнего уровня

    // Тело проверяется как функция верхнего уровня, как копия шаблона
    std::vector<Context> callerContexts(contexts.begin() + 1, contexts.end());
    contexts.resize(1);
    const int outerLoopDepth = loopDepth;
    const bool outerInMethod = insideMethod, outerIndexedString = lastAccessIndexedString;
    loopDepth = 0;
    insideMethod = false;
    lifted->accept(*this);
    loopDepth = outerLoopDepth;
    insideMethod = outerInMethod;
    lastAccessIndexedString = outerIndexedString;
    contexts.insert(contexts.end(), callerContexts.begin(), callerContexts.end());

    node.inferredType = functionTypeOf(*lifted);
}
