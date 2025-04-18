#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::visit(BinaryOpNode& node) {
    node.left->accept(*this);
    node.right->accept(*this);

    std::shared_ptr<TypeNode> leftType = node.left->inferredType;
    std::shared_ptr<TypeNode> rightType = node.right->inferredType;
    
    // "+"
    if (node.op == "+" || node.op == "add" || node.op == "fadd") {
        handlePlusOperator(node, leftType, rightType);
        node.inferredType = node.left->inferredType;
        return;
    }

    // "-"
    if (node.op == "-" || node.op == "sub" || node.op == "fsub") {
        handleMinusOperator(node, leftType, rightType);
        node.inferredType = node.left->inferredType;
        return;
    }

    // "*"
    if (node.op == "*" || node.op == "mul" || node.op == "fmul") {
        handleMulOperator(node, leftType, rightType);
        node.inferredType = node.left->inferredType;
        return;
    }

    // "/"
    if (node.op == "/" || node.op == "sdiv" || node.op == "fdiv") {
        handleDivOperator(node, leftType, rightType);
        node.inferredType = node.left->inferredType;
        return;
    }

    // "%"
    if (node.op == "%" || node.op == "srem" || node.op == "frem") {
        handleModOperator(node, leftType, rightType);
        node.inferredType = node.left->inferredType;
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
        handleCompareOperator(node, leftType, rightType);
        node.inferredType = registry.findType("i1");
        
        return;
    }

    // and, or 
    if (node.op == "and" || node.op == "or") {
        handleLogicalOperator(node, leftType, rightType);
        node.inferredType = registry.findType("i1");
        return;
    }

    IC();
}

void TypeSymbolVisitor::visit(UnaryOpNode& node) {
    node.operand->accept(*this);
    std::shared_ptr<TypeNode> operandType = node.operand->inferredType;
    node.inferredType = operandType;
    /*
    if (node.op == "-" || node.op == "sub") {
        handleUnaryOperator(node, operandType);
        node.inferredType = operandType;
        return;
    }

    if (node.op == "!") {
        handleLogicalOperator(node, operandType, nullptr);
        node.inferredType = registry.findType("i1");
        return;
    }
    */
    IC();
}
