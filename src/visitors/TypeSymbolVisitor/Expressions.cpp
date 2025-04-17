#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::visit(BinaryOpNode& node) {
    // Рекурсивно анализируем левый и правый операнд
    node.left->accept(*this);
    node.right->accept(*this);

    IC();

    std::shared_ptr<TypeNode> leftType = checkForIdentifier(node.left);
    std::shared_ptr<TypeNode> rightType = checkForIdentifier(node.right);

    IC(leftType->toString(), rightType->toString());
    // Пример для оператора "+"
    if (node.op == "+") {

        // Если оба операнда — одного типа
        if (leftType->toString() == rightType->toString() && leftType->toString() != "string") {
            if (leftType->toString() == "i32" 
            || leftType->toString() == "i64" 
            || leftType->toString() == "i8" 
            || leftType->toString() == "i1") { node = *std::make_shared<BinaryOpNode>(node.left, "add", node.right); } 
            else if (
            leftType->toString() == "float") { node = *std::make_shared<BinaryOpNode>(node.left, "fadd", node.right); }
            else {
                // Cюда буду
                LogError("Unsupported operand types for '+': " + leftType->toString() + " and " + rightType->toString());
            }

            node.inferredType = leftType;
            return;
        }

        // Если хотя бы один — строка, делаем toString для второго и strcat
        if (leftType->toString() == "string" || rightType->toString() == "string") {
            if (leftType->toString() != "string") {
                node.left = std::make_shared<CallNode>("toString", std::vector{node.left});
                node.left->accept(*this);
            }
            if (rightType->toString() != "string") {
                node.right = std::make_shared<CallNode>("toString", std::vector{node.right});
                node.right->accept(*this);
            }
            // После преобразования оба string — подставляем strcat
            node = *std::make_shared<BinaryOpNode>(node.left, "strcat", node.right); // или CallNode("strcat", ...)
            node.inferredType = registry.findType("string");
            return;
        }

        if (leftType->toString() == "float" || rightType->toString() == "float") {
            if (
               leftType->toString() == "i32" 
            || leftType->toString() == "i64" 
            || leftType->toString() == "i8" 
            || leftType->toString() == "bool"
            ) {
                node.left->implicitCastTo = registry.findType("float");
            }
            if (
                rightType->toString() == "i32" 
                || rightType->toString() == "i64" 
                || rightType->toString() == "i8"
                || rightType->toString() == "bool"
            ) {
                node.right->implicitCastTo = registry.findType("float");
            }
            node = *std::make_shared<BinaryOpNode>(node.left, "fadd", node.right);
            node.inferredType = registry.findType("float");
            return;
        }

        numCast(node.left, node.right, node.op);
        node.inferredType = node.left->inferredType;
        std::cout << "numCast: finished BinaryOpNode block" << std::endl;
        return;
    }
    // ...обработка других операторов
    IC();
}