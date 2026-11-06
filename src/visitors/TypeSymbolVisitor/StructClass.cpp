#include "../headers/TypeSymbolVisitor.h"

void TypeSymbolVisitor::visit(StructNode& node) {
    // Проверяем, существует ли структура в реестре
    // Сбор объявлений уже положил сюда эту же структуру, повтор это другая структура с тем же именем
    auto existingStruct = registry.findStruct(node.name);
    if (existingStruct && existingStruct->body != node.body) {
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

    registry.addStruct(node.name, std::make_shared<StructNode>(node.name, node.body));

    // Обрабатываем тело структуры
    if (auto blockNode = std::dynamic_pointer_cast<BlockNode>(node.body)) {
        for (const auto& statement : blockNode->statements) {
            // Конструкторов пока нет, поэтому значение полю взять неоткуда. [] и {} это значение
            auto field = std::dynamic_pointer_cast<VariableAssignNode>(statement);
            auto noValue = field ? std::dynamic_pointer_cast<NoneNode>(field->expression) : nullptr;
            if (noValue && !noValue->isEmptyLiteral) {
                LogError("Field '" + field->name + "' of struct '" + node.name + "' must have a default value", statement);
            }

            statement->accept(*this);
        }
    }

    // Добавляем структуру в реестр
    //registry.addType(node.name, std::make_shared<GenericTypeNode>(node.name, std::vector<std::shared_ptr<TypeNode>>{}));

#if DEBUG
    debugContexts();
#endif
    // Возвращаемся к предыдущему контексту
    contexts.pop_back();
}