#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::visit(StructNode& node) {
    // Проверяем, существует ли структура в реестре
    if (registry.findStruct(node.name)) {
        LogError("Struct already defined: " + node.name, node.shared_from_this());
    }

    // Создаем новый контекст для структуры
    Context structContext = {
        .labels = {},
        .variables = {},
        .functions = {},
        .currentFunctionName = node.name,
        .returnType = nullptr
    };

    contexts.push_back(structContext);

    // Обрабатываем тело структуры
    if (auto blockNode = std::dynamic_pointer_cast<BlockNode>(node.body)) {
        for (const auto& statement : blockNode->statements) {
            statement->accept(*this);
        }
    }

    // Добавляем структуру в реестр
    //registry.addType(node.name, std::make_shared<GenericTypeNode>(node.name, std::vector<std::shared_ptr<TypeNode>>{}));
    registry.addStruct(node.name, std::make_shared<StructNode>(node.name, node.body));

    debugContexts();
    // Возвращаемся к предыдущему контексту
    contexts.pop_back();
}