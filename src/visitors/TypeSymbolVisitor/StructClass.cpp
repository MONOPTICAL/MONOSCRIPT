#include "../headers/TypeSymbolVisitor.h"
#include <algorithm>
#include <set>

// Какие структуры обязательно создаются при создании этой: поле такого типа или вызов P() в значении поля
static void collectConstructedStructs(const std::shared_ptr<ASTNode>& expression, Registry& registry, std::vector<std::string>& result) {
    if (auto call = std::dynamic_pointer_cast<CallNode>(expression)) {
        if (registry.findStruct(call->callee)) result.push_back(call->callee);
        for (const auto& argument : call->arguments) collectConstructedStructs(argument, registry, result);
    } else if (auto block = std::dynamic_pointer_cast<BlockNode>(expression)) {
        for (const auto& element : block->statements) collectConstructedStructs(element, registry, result);
    }
}

static bool constructsItself(const std::string& name, const std::string& target, Registry& registry, std::vector<std::string>& visited) {
    if (std::find(visited.begin(), visited.end(), name) != visited.end()) return false;
    visited.push_back(name);

    auto structure = registry.findStruct(name);
    auto body = structure ? std::dynamic_pointer_cast<BlockNode>(structure->body) : nullptr;
    if (!body) return false;

    for (const auto& statement : body->statements) {
        auto field = std::dynamic_pointer_cast<VariableAssignNode>(statement);
        if (!field) continue;

        std::vector<std::string> constructed;
        if (field->type && registry.findStruct(field->type->toString())) constructed.push_back(field->type->toString());
        collectConstructedStructs(field->expression, registry, constructed);

        for (const auto& other : constructed)
            if (other == target || constructsItself(other, target, registry, visited)) return true;
    }
    return false;
}

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

    std::string path = contexts.back().scopePath;
    std::string fullName = !path.empty() ? path + "." + node.name : node.name;
    if (contexts.back().structsFullName.count(node.name)) LogError("Struct already defined: " + node.name);

    contexts.back().structsFullName.emplace(node.name, fullName);
    node.name = fullName;

    std::cout << "Processing struct: " << fullName << std::endl;
    std::cout << "Current context: " << contexts.back().currentFunctionName << std::endl;
    contexts.push_back(structContext);

    registry.addStruct(fullName, std::make_shared<StructNode>(node.name, node.body));

    // Обрабатываем тело структуры
    if (auto blockNode = std::dynamic_pointer_cast<BlockNode>(node.body)) {
        // Цепочка ищет член структуры по имени, поэтому имена не должны повторяться
        std::set<std::string> members;
        for (const auto& statement : blockNode->statements) {
            std::string member;
            if (auto field = std::dynamic_pointer_cast<VariableAssignNode>(statement)) member = field->name;
            else if (auto method = std::dynamic_pointer_cast<FunctionNode>(statement)) member = method->name;
            if (!member.empty() && !members.insert(member).second)
                LogError("Struct '" + node.name + "' already has a member named '" + member + "'", statement);
        }

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

    // Ссылка не бывает пустой, поэтому такую структуру создать невозможно: создание никогда не закончится.
    // Массив с пустым значением по умолчанию это не цикл: array<Node> kids = []
    std::vector<std::string> visited;
    if (constructsItself(node.name, node.name, registry, visited)) {
        LogError("Struct '" + node.name + "' cannot be created: its fields always create another " + node.name, node.shared_from_this());
    }

    // Добавляем структуру в реестр
    //registry.addType(node.name, std::make_shared<GenericTypeNode>(node.name, std::vector<std::shared_ptr<TypeNode>>{}));

#if DEBUG
    debugContexts();
#endif
    // Возвращаемся к предыдущему контексту
    contexts.pop_back();

    if (contexts.size() > 1) hoistedStructs.push_back(node.shared_from_this());
}