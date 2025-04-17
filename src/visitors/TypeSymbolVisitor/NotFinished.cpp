#include "../headers/TypeSymbolVisitor.h"

// Заглушки для visit-методов

void TypeSymbolVisitor::visit(StructNode& node) {
    // TODO: реализовать обработку StructNode
}

void TypeSymbolVisitor::visit(ReassignMemberNode& node) {
    // TODO: реализовать обработку ReassignMemberNode
}

void TypeSymbolVisitor::visit(VariableReassignNode& node) {
    // TODO: реализовать обработку VariableReassignNode
}

void TypeSymbolVisitor::visit(IfNode& node) {
    // TODO: реализовать обработку IfNode
}

void TypeSymbolVisitor::visit(ForNode& node) {
    // TODO: реализовать обработку ForNode
}

void TypeSymbolVisitor::visit(WhileNode& node) {
    // TODO: реализовать обработку WhileNode
}

void TypeSymbolVisitor::visit(CallNode& node) {
    // TODO: реализовать обработку CallNode
}

void TypeSymbolVisitor::visit(UnaryOpNode& node) {
    // TODO: реализовать обработку UnaryOpNode
}

void TypeSymbolVisitor::visit(BreakNode& node) {
    // TODO: реализовать обработку BreakNode
}

void TypeSymbolVisitor::visit(ContinueNode& node) {
    // TODO: реализовать обработку ContinueNode
}

void TypeSymbolVisitor::visit(AccessExpression& node) {
    // TODO: реализовать обработку AccessExpression
}

void TypeSymbolVisitor::visit(ClassNode& node) {
    // TODO: реализовать обработку ClassNode
    /*
    сделай перед проверкой блоков(public, private) пустой контекст(это будет как общий контекст для public и private)
    потому что классы никак не привязаны к внешнему контексту
    */
}