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
        handlePlusOperator(node, leftType, rightType);
        //numCast(node.left, node.right, node.op);
        node.inferredType = node.left->inferredType;
        std::cout << "numCast: finished BinaryOpNode block" << std::endl;
        return;
    }
    // ...обработка других операторов
    IC();
}