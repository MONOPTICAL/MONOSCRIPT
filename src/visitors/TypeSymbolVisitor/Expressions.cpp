#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::visit(BinaryOpNode& node) {
    node.left->accept(*this);
    node.right->accept(*this);

    std::shared_ptr<TypeNode> leftType = node.left->inferredType;
    std::shared_ptr<TypeNode> rightType = node.right->inferredType;
    
    std::shared_ptr<BinaryOpNode> root = std::make_shared<BinaryOpNode>(node.left, node.op, node.right);
   
    // "+"
    if (node.op == "+" || node.op == "add" || node.op == "fadd") {
        handlePlusOperator(root, leftType, rightType);
        node = *root;
        return;
    }

    // "-"
    if (node.op == "-" || node.op == "sub" || node.op == "fsub") {
        handlePlusOperator(root, leftType, rightType);
        node = *root;
        return;
    }

    // "*"
    if (node.op == "*" || node.op == "mul" || node.op == "fmul") {
        handleMulOperator(root, leftType, rightType);
        node = *root;
        return;
    }

    // "/"
    if (node.op == "/" || node.op == "sdiv" || node.op == "fdiv") {
        handleDivOperator(root, leftType, rightType);
        node = *root;
        return;
    }

    // "%"
    if (node.op == "%" || node.op == "srem" || node.op == "frem") {
        handleModOperator(root, leftType, rightType);
        node = *root;
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
        node = *root;
        return;
    }

    // and, or 
    if (node.op == "and" || node.op == "or") {
        handleLogicalOperator(root, leftType, rightType);
        node = *root;
        return;
    }
}

void TypeSymbolVisitor::visit(UnaryOpNode& node) {
    node.operand->accept(*this);
    std::shared_ptr<TypeNode> operandType = node.operand->inferredType;

    if (node.op == "?")
    {
        if (std::dynamic_pointer_cast<CallNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<AccessExpression>(node.operand)) {}
        else if (std::dynamic_pointer_cast<IdentifierNode>(node.operand)) {}
        else if (std::dynamic_pointer_cast<LambdaNode>(node.operand)) {}
        else
        {
            LogError("Unsupported operand type for '?'");
            return;
        }
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
            LogError("Unsupported operand type for '!'");
            return;
        }
    }

    node.inferredType = operandType;
}
