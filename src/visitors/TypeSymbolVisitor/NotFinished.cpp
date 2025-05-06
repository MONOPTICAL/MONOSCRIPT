#include "../headers/TypeSymbolVisitor.h"

// Заглушки для visit-методов

void TypeSymbolVisitor::visit(ReassignMemberNode& node) {
    // x.y = 1
    // TODO: реализовать обработку ReassignMemberNode
}

void TypeSymbolVisitor::visit(AccessExpression& node) {
    // TODO: реализовать обработку AccessExpression
    // x[1], x.y 
}

void TypeSymbolVisitor::visit(ImportNode &node)
{
    // Не надо его реализовывать, он обрабатывается в другом месте(при загрузке модулей в AST)
}