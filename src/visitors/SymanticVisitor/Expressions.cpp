#include "../headers/SymanticVisitor.h"

void SymanticVisitor::visit(BinaryOpNode& node) {
    // Рекурсивно анализируем левый и правый операнд
    node.left->accept(*this);
    node.right->accept(*this);


    auto leftType = checkForIdentifier(node.left);
    auto rightType = checkForIdentifier(node.right);

    // Пример для оператора "+"
    if (node.op == "+") {

        // Если оба операнда — одного типа
        if (leftType->toString() == rightType->toString() && leftType->toString() != "string") {
            if (leftType->toString() == "i32" 
            || leftType->toString() == "i64" 
            || leftType->toString() == "i8" 
            || leftType->toString() == "bool") { node = *std::make_shared<BinaryOpNode>(node.left, "add", node.right); } 
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
        // ...другие случаи (например, сложение float и int)
        LogError("Unsupported operand types for '+': " + leftType->toString() + " and " + rightType->toString());
    }
    // ...обработка других операторов
}