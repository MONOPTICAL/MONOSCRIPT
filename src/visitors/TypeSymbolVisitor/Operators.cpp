#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::handlePlusOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    // string
    if (left == "string" || right == "string") {

        if (left != "string") {
            node->left = std::make_shared<CallNode>("toString", std::vector{node->left});
            node->left->accept(*this);
        }
        if (right != "string") {
            node->right = std::make_shared<CallNode>("toString", std::vector{node->right});
            node->right->accept(*this);
        }
        node = std::make_shared<BinaryOpNode>(node->left, "strcat", node->right);
        node->inferredType = registry.findType("string");
        return;
    }

    // float
    if (left == "float" || right == "float") {
        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        node = std::make_shared<BinaryOpNode>(node->left, "fadd", node->right);
        node->inferredType = registry.findType("float");
        return;
    }

    // int(i32, i64, i8, i1)
    if (isIntType(left) && isIntType(right)) {
        node = std::make_shared<BinaryOpNode>(node->left, "add", node->right);
        node->inferredType = leftType;
        return;
    }

    LogError("Unsupported operand types for '+': " + left + " and " + right);
}

void TypeSymbolVisitor::handleMinusOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{

    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    // string
    if (left == "string" || right == "string") {
        LogError("Unsupported operand types for '-': " + left + " and " + right);
        return;
    }

    // float
    if (left == "float" || right == "float") {
        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        node = std::make_shared<BinaryOpNode>(node->left, "fsub", node->right);
        node->inferredType = registry.findType("float");
        return;
    }

    // int(i32, i64, i8, i1)
    if (isIntType(left) && isIntType(right)) {
        node = std::make_shared<BinaryOpNode>(node->left, "sub", node->right);
        node->inferredType = leftType;
        return;
    }

    LogError("Unsupported operand types for '+': " + left + " and " + right);
}

void TypeSymbolVisitor::handleMulOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    // string
    if (left == "string" || right == "string") {
        LogError("Unsupported operand types for '*': " + left + " and " + right);
        return;
    }

    // float
    if (left == "float" || right == "float") {
        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        node = std::make_shared<BinaryOpNode>(node->left, "fmul", node->right);
        node->inferredType = registry.findType("float");
        return;
    }

    // int(i32, i64, i8, i1)
    if (isIntType(left) && isIntType(right)) {
        node = std::make_shared<BinaryOpNode>(node->left, "mul", node->right);
        node->inferredType = leftType;
        return;
    }

    LogError("Unsupported operand types for '+': " + left + " and " + right);
}

void TypeSymbolVisitor::handleDivOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    if (left == "string" || right == "string") {
        LogError("Unsupported operand types for '/': " + left + " and " + right);
        return;
    }

    if (left == "float" || right == "float") {
        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        node = std::make_shared<BinaryOpNode>(node->left, "fdiv", node->right);
        node->inferredType = registry.findType("float");
        return;
    }

    if (isIntType(left) && isIntType(right)) {
        node = std::make_shared<BinaryOpNode>(node->left, "sdiv", node->right);
        node->inferredType = leftType;
        return;
    }

    LogError("Unsupported operand types for '/': " + left + " and " + right);
}

void TypeSymbolVisitor::handleModOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i8" || t == "i1";
    };

    if (left == "string" || right == "string") {
        LogError("Unsupported operand types for '%': " + left + " and " + right);
        return;
    }

    if (left == "float" || right == "float") {
        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        node = std::make_shared<BinaryOpNode>(node->left, "frem", node->right);
        node->inferredType = registry.findType("float");
        return;
    }

    if (isIntType(left) && isIntType(right)) {
        node = std::make_shared<BinaryOpNode>(node->left, "srem", node->right);
        node->inferredType = leftType;
        return;
    }

    LogError("Unsupported operand types for '%': " + left + " and " + right);
}

void TypeSymbolVisitor::handleCompareOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i8" || t == "i1";
    };

    // IR-операции по node->op
    std::string cmpOp;
    if (node->op == "==" || node->op == "eq") cmpOp = "eq";
    else if (node->op == "!=" || node->op == "ne") cmpOp = "ne";
    else if (node->op == "<") cmpOp = "slt";
    else if (node->op == ">") cmpOp = "sgt";
    else if (node->op == "<=") cmpOp = "sle";
    else if (node->op == ">") cmpOp = "sgt";
    else if (node->op == ">=") cmpOp = "sge";
    else cmpOp = ""; // неизвестный оператор

    if (cmpOp.empty()) {
        LogError("Unknown comparison operator: " + node->op);
        return;
    }

    // string
    if (left == "string" || right == "string") {
        node = std::make_shared<BinaryOpNode>(node->left, "strcmp_" + cmpOp, node->right);
        node->inferredType = registry.findType("i1");
        return;
    }

    // float
    if (left == "float" || right == "float") {
        if (left != "float") node->left->implicitCastTo = registry.findType("float");
        if (right != "float") node->right->implicitCastTo = registry.findType("float");
        node = std::make_shared<BinaryOpNode>(node->left, "fcmp_" + cmpOp, node->right);
        node->inferredType = registry.findType("i1");
        return;
    }

    // int(i32, i64, i8, i1)
    if (isIntType(left) && isIntType(right)) {
        node = std::make_shared<BinaryOpNode>(node->left, "icmp_" + cmpOp, node->right);
        node->inferredType = registry.findType("i1");
        return;
    }

    LogError("Unsupported operand types for comparison: " + left + " and " + right);
}

void TypeSymbolVisitor::handleLogicalOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    if (left != "i1" || right != "i1") {
        LogError("Logical operators require boolean operands, got: " + left + " and " + right);
        return;
    }

    // and, or
    std::string op = node->op;
    if (op == "and") op = "and";
    else if (op == "or") op = "or";
    else {
        LogError("Unknown logical operator: " + node->op);
        return;
    }

    node = std::make_shared<BinaryOpNode>(node->left, op, node->right);
    node->inferredType = registry.findType("i1");
    node->implicitCastTo = registry.findType("i1");
}