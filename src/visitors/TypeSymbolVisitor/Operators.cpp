#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::handlePlusOperator(BinaryOpNode &node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i8" || t == "i1" ;
    };

    // Если один из операндов string — конкатенация строк
    if (left == "string" || right == "string") {
        if (left != "string") {
            node.left = std::make_shared<CallNode>("toString", std::vector{node.left});
            node.left->accept(*this);
        }
        if (right != "string") {
            node.right = std::make_shared<CallNode>("toString", std::vector{node.right});
            node.right->accept(*this);
        }
        node = *std::make_shared<BinaryOpNode>(node.left, "strcat", node.right);
        node.inferredType = registry.findType("string");
        return;
    }

    // Если один из операндов float — сложение с приведением к float
    if (left == "float" || right == "float") {
        if (left != "float") {
            node.left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node.right->implicitCastTo = registry.findType("float");
        }
        node = *std::make_shared<BinaryOpNode>(node.left, "fadd", node.right);
        node.inferredType = registry.findType("float");
        return;
    }

    // Если оба операнда целые — обычное сложение
    if (isIntType(left) && isIntType(right)) {
        node = *std::make_shared<BinaryOpNode>(node.left, "add", node.right);
        node.inferredType = leftType;
        return;
    }

    LogError("Unsupported operand types for '+': " + left + " and " + right);
}