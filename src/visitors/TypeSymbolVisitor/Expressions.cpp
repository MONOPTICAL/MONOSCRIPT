#include "../headers/TypeSymbolVisitor.h"

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

    std::shared_ptr<BinaryOpNode> root = std::make_shared<BinaryOpNode>(node.left, node.op, node.right);
    root->implicitCastTo = node.implicitCastTo; // Явный каст (a + b)->i32 не должен потеряться
    root->line = node.line; root->column = node.column;

    // Обработчики операторов создают новый узел, а место в исходном коде остаётся прежним
    auto finish = [&]() {
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
    if (node.op == "/" || node.op == "sdiv" || node.op == "fdiv") {
        handleDivOperator(root, leftType, rightType);
        finish();
        return;
    }

    // "%"
    if (node.op == "%" || node.op == "srem" || node.op == "frem") {
        handleModOperator(root, leftType, rightType);
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
    if (node.op == "**" || node.op == "|>")
        LogError("Operator '" + node.op + "' is not implemented yet", node.right);
}

void TypeSymbolVisitor::visit(UnaryOpNode& node) {
    node.operand->accept(*this);
    std::shared_ptr<TypeNode> operandType = node.operand->inferredType;

    // char не число: сначала каст
    if (operandType && operandType->toString() == "char" && !node.operand->implicitCastTo)
        LogError("Unsupported operand type for unary operator: char, use ->i32", node.shared_from_this());

    // Узел уже обходили и переименовали оператор, второй обход ничего не меняет
    if (node.op == "neg" || node.op == "not" || node.op == "nullcheck")
    {
        node.inferredType = operandType;
        return;
    }

    if (node.op == "?")
    {
        LogError("Operator '?' is not implemented yet", node.shared_from_this());
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
    LogError("Lambdas are not implemented yet", node.shared_from_this());
}