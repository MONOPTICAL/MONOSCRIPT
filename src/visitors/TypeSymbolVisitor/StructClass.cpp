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

void TypeSymbolVisitor::instantiateStruct(const std::string& name)
{
    const auto open = name.find('<');
    if (open == std::string::npos) return;
    auto found = genericStructs.find(name.substr(0, open));
    if (found == genericStructs.end() || registry.findStruct(name)) return;
    const auto& generic = found->second;

    // Аргументы до верхней запятой: Pair<map<string, i32>,u8> это map<string, i32> и u8
    std::vector<std::string> arguments;
    int depth = 0;
    std::string current;
    for (char c : name.substr(open + 1, name.size() - open - 2)) {
        if (c == '<') depth++;
        if (c == '>') depth--;
        if (c == ',' && depth == 0) { arguments.push_back(current); current.clear(); continue; }
        current += c;
    }
    arguments.push_back(current);
    if (arguments.size() != generic->typeParameters.size()) {
        LogError("Struct " + generic->name + " expects " + std::to_string(generic->typeParameters.size()) + " type arguments, got " + std::to_string(arguments.size()));
        return;
    }

    std::map<std::string, std::string> types;
    for (size_t i = 0; i < arguments.size(); i++) types[generic->typeParameters[i]] = arguments[i];
    auto instance = generic->instantiate(types);

    // Поле типа T без значения получает нулевое значение своего типа: ссылка всё равно не бывает пустой
    if (auto block = std::dynamic_pointer_cast<BlockNode>(instance->body))
        for (auto& statement : block->statements) {
            auto field = std::dynamic_pointer_cast<VariableAssignNode>(statement);
            auto none = field ? std::dynamic_pointer_cast<NoneNode>(field->expression) : nullptr;
            if (!none || none->isEmptyLiteral) continue;
            const std::string type = field->type->toString();
            if (isIntegerTypeName(type)) field->expression = std::make_shared<NumberNode>(0, std::make_shared<SimpleTypeNode>("i1"));
            else if (isFloatType(type)) field->expression = std::make_shared<FloatNumberNode>(0.0);
            else if (type == "string") field->expression = std::make_shared<StringNode>("");
            else if (type.starts_with("array<") || type.starts_with("map<")) none->isEmptyLiteral = true;
            else if (registry.findStruct(type) || type.find('<') != std::string::npos) field->expression = std::make_shared<CallNode>(type, std::vector<std::shared_ptr<ASTNode>>{});
            if (field->expression != none) { field->expression->line = field->line; field->expression->column = field->column; }
        }

    // Копия проверяется как структура верхнего уровня, а не внутри того, кто её упомянул
    std::vector<Context> callerContexts(contexts.begin() + 1, contexts.end());
    contexts.resize(1);
    const std::string outerNamespace = contexts[0].currentNamespace, outerScopePath = contexts[0].scopePath;
    contexts[0].currentNamespace = contexts[0].scopePath = "";
    instance->accept(*this);
    contexts[0].currentNamespace = outerNamespace;
    contexts[0].scopePath = outerScopePath;
    contexts.insert(contexts.end(), callerContexts.begin(), callerContexts.end());
    hoistedStructs.push_back(instance); // Кодгену копия нужна как обычная структура верхнего уровня
}

void TypeSymbolVisitor::visit(StructNode& node) {
    if (!node.typeParameters.empty()) return; // Проверяются только копии шаблона
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

    if (contexts.size() > 1 || !contexts.back().currentNamespace.empty()) hoistedStructs.push_back(node.shared_from_this());
}