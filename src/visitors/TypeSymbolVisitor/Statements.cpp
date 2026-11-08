#include <algorithm>
#include "../headers/TypeSymbolVisitor.h"
#include "../../CLI/headers/ui.h"
#include "../../errors/headers/ErrorEngine.h"
#include <iostream>

std::shared_ptr<ASTNode> toStringHandler(std::shared_ptr<ASTNode>& node, TypeSymbolVisitor& visitor);
std::shared_ptr<TypeNode> functionTypeOf(const FunctionNode& function);
bool isLiteralExpression(const std::shared_ptr<ASTNode>& node);

// Упоминается ли имя в типе: T в array<map<string,T>>
static bool typeMentions(const std::shared_ptr<TypeNode>& type, const std::string& name) {
    if (auto simple = std::dynamic_pointer_cast<SimpleTypeNode>(type)) return simple->name == name;
    if (auto generic = std::dynamic_pointer_cast<GenericTypeNode>(type))
        for (const auto& parameter : generic->typeParameters)
            if (typeMentions(parameter, name)) return true;
    return false;
}

// Тип литерала без контекста: i32, а не наименьший подходящий, иначе id(1) было бы i1. Вещественный это f64
static std::shared_ptr<TypeNode> widenLiteralType(const std::shared_ptr<TypeNode>& type) {
    if (auto generic = std::dynamic_pointer_cast<GenericTypeNode>(type)) {
        auto widened = std::make_shared<GenericTypeNode>(generic->baseName);
        for (const auto& parameter : generic->typeParameters)
            widened->typeParameters.push_back(widenLiteralType(parameter));
        return widened;
    }
    const std::string name = type->toString();
    if (name == "i1" || name == "i8" || name == "i16") return std::make_shared<SimpleTypeNode>("i32");
    if (name == "float") return std::make_shared<SimpleTypeNode>("f64");
    return type;
}

// Тип литерала коллекции для вывода T. Ничего не меняет в узлах: приведёт уже проверка вызова с готовым типом
static std::shared_ptr<TypeNode> literalCollectionType(const std::shared_ptr<BlockNode>& block);

static std::shared_ptr<TypeNode> literalValueType(const std::shared_ptr<ASTNode>& value, TypeSymbolVisitor& visitor) {
    if (auto block = std::dynamic_pointer_cast<BlockNode>(value)) return literalCollectionType(block);
    value->accept(visitor);
    return value->implicitCastTo ? value->implicitCastTo : value->inferredType;
}

// Настоящий тип важнее литералов, среди литералов берём более широкий
static std::shared_ptr<TypeNode> mergeElementType(const std::shared_ptr<TypeNode>& current, const std::shared_ptr<TypeNode>& next, bool nextIsLiteral, bool& haveValue) {
    if (!next) return current;
    if (!nextIsLiteral) {
        if (!haveValue) { haveValue = true; return next; }
        return current;
    }
    if (haveValue) return current;
    auto widened = widenLiteralType(next);
    if (!current) return widened;
    const std::string a = current->toString(), b = widened->toString();
    if (a == "f64" || b == "f64") return std::make_shared<SimpleTypeNode>("f64");
    if (a == "i64" || b == "i64") return std::make_shared<SimpleTypeNode>("i64");
    return current;
}

// Сопоставляет тип параметра с типом аргумента и находит T. Несовпадение остального проверит обычная проверка вызова
static void unifyTypes(const std::shared_ptr<TypeNode>& parameter, const std::shared_ptr<TypeNode>& argument,
                       const std::vector<std::string>& names, std::map<std::string, std::string>& bindings,
                       bool fromLiteral, std::string& conflict) {
    if (!parameter || !argument) return;

    if (auto simple = std::dynamic_pointer_cast<SimpleTypeNode>(parameter)) {
        if (std::find(names.begin(), names.end(), simple->name) == names.end()) return;

        auto bound = bindings.find(simple->name);
        if (bound == bindings.end())
            bindings[simple->name] = argument->toString();
        else if (!fromLiteral && bound->second != argument->toString())
            conflict = simple->name + " is both " + bound->second + " and " + argument->toString();
        return;
    }

    auto generic = std::dynamic_pointer_cast<GenericTypeNode>(parameter);
    auto other = std::dynamic_pointer_cast<GenericTypeNode>(argument);
    if (generic && other && generic->baseName == other->baseName && generic->typeParameters.size() == other->typeParameters.size())
        for (size_t i = 0; i < generic->typeParameters.size(); i++)
            unifyTypes(generic->typeParameters[i], other->typeParameters[i], names, bindings, fromLiteral, conflict);
}

static TypeSymbolVisitor* literalVisitor = nullptr;

static std::shared_ptr<TypeNode> literalCollectionType(const std::shared_ptr<BlockNode>& block) {
    if (!block || block->statements.empty()) return nullptr;

    if (std::dynamic_pointer_cast<KeyValueNode>(block->statements[0])) {
        std::shared_ptr<TypeNode> key, value;
        bool haveKey = false, haveValue = false;
        for (const auto& statement : block->statements) {
            auto pair = std::dynamic_pointer_cast<KeyValueNode>(statement);
            if (!pair) return nullptr;
            key = mergeElementType(key, literalValueType(pair->key, *literalVisitor), isLiteralExpression(pair->key), haveKey);
            value = mergeElementType(value, literalValueType(pair->value, *literalVisitor), isLiteralExpression(pair->value), haveValue);
        }
        if (!key || !value) return nullptr;
        auto map = std::make_shared<GenericTypeNode>("map");
        map->typeParameters = {key, value};
        return map;
    }

    std::shared_ptr<TypeNode> element;
    bool haveValue = false;
    for (const auto& statement : block->statements) {
        element = mergeElementType(element, literalValueType(statement, *literalVisitor), isLiteralExpression(statement), haveValue);
    }
    if (!element) return nullptr;
    auto array = std::make_shared<GenericTypeNode>("array");
    array->typeParameters = {element};
    return array;
}

std::string TypeSymbolVisitor::instantiateGeneric(const std::shared_ptr<FunctionNode>& generic, CallNode& call,
                                                  const std::vector<std::shared_ptr<TypeNode>>& argTypes)
{
    if (call.arguments.size() != generic->parameters.size()) {
        LogError("Function " + generic->name + " expects " + std::to_string(generic->parameters.size()) + " arguments, got " + std::to_string(call.arguments.size()), call.shared_from_this());
    }

    // Сначала значения с настоящим типом, потом литералы: в max(big, 1) при i64 big литерал подстроится
    std::map<std::string, std::string> bindings;
    std::string conflict;
    for (size_t i = 0; i < call.arguments.size(); i++) {
        if (!isLiteralExpression(call.arguments[i]) && !std::dynamic_pointer_cast<BlockNode>(call.arguments[i]))
            unifyTypes(generic->parameters[i].first, argTypes[i], generic->typeParameters, bindings, false, conflict);
    }
    if (!conflict.empty()) {
        LogError("Cannot call " + generic->name + ": " + conflict, call.shared_from_this());
    }
    for (size_t i = 0; i < call.arguments.size(); i++) {
        std::shared_ptr<TypeNode> literalType;
        if (auto block = std::dynamic_pointer_cast<BlockNode>(call.arguments[i])) {
            literalVisitor = this;
            literalType = literalCollectionType(block);
        } else if (isLiteralExpression(call.arguments[i])) {
            literalType = argTypes[i];
        }
        if (literalType)
            {
            auto number = std::dynamic_pointer_cast<NumberNode>(call.arguments[i]);
            unifyTypes(generic->parameters[i].first, number && number->isBoolLiteral ? literalType : widenLiteralType(literalType),
                       generic->typeParameters, bindings, true, conflict);
        }
    }

    std::string suffix;
    for (const auto& name : generic->typeParameters) {
        auto bound = bindings.find(name);
        if (bound == bindings.end()) {
            LogError("Cannot infer type " + name + " of " + generic->name + " from its arguments", call.shared_from_this());
        }
        suffix += (suffix.empty() ? "" : ",") + bound->second;
    }
    suffix.erase(std::remove(suffix.begin(), suffix.end(), ' '), suffix.end());

    const std::string instanceName = generic->name + "<" + suffix + ">";
    if (declaredFunctions.count(instanceName)) return instanceName;

    auto instance = generic->instantiate(bindings);
    instance->name = instanceName;
    instance->inferredType = instance->returnType;
    declaredFunctions[instanceName] = instance; // До проверки тела: копия может вызывать сама себя
    genericInstances.push_back(instance);

    // Тело проверяется как функция верхнего уровня, а не как часть той функции, откуда вызвали
    std::vector<Context> callerContexts(contexts.begin() + 1, contexts.end());
    contexts.resize(1);
    const int outerLoopDepth = loopDepth;
    const bool outerInMethod = insideMethod;
    const bool outerIndexedString = lastAccessIndexedString;
    const std::string outerModule = currentModuleName;
    loopDepth = 0;
    insideMethod = false;
    if (auto module = genericModules.find(generic->name); module != genericModules.end())
        currentModuleName = module->second;

    try {
        instance->accept(*this);
    } catch (const std::exception&) {
        // Ошибка в теле уже напечатана: добавляем, откуда взялись такие типы
        std::string description;
        for (const auto& name : generic->typeParameters)
            description += (description.empty() ? "" : ", ") + name + " = " + bindings[name];
        std::cerr << "    in generic function " << generic->name << " with " << description
                  << ", called at line " << ErrorEngine::getInstance().displayLine(call.line) << std::endl;
        throw;
    }

    contexts.insert(contexts.end(), callerContexts.begin(), callerContexts.end());
    loopDepth = outerLoopDepth;
    insideMethod = outerInMethod;
    lastAccessIndexedString = outerIndexedString;
    currentModuleName = outerModule;
    return instanceName;
}
#include "../../includes/ASTDebugger.hpp"
#include <string_view>
#include <filesystem>

// Путь файла -> имя модуля, как в use: mono/std/a/b.ms -> std.a.b, dir/lib.ms -> lib.
// Уже готовое имя (std.a.b, lib) возвращается как есть
std::string moduleNameOf(const std::string& path) {
    if (!path.ends_with(".ms") && path.find('/') == std::string::npos) return path;
    const auto file = std::filesystem::weakly_canonical(path);
    const auto std = std::filesystem::weakly_canonical(STDLIB_MS_PATH);
    auto relative = file.lexically_relative(std);
    if (!relative.empty() && *relative.begin() != "..") {
        std::string name = relative.replace_extension().generic_string();
        std::replace(name.begin(), name.end(), '/', '.');
        return "std." + name;
    }
    return file.stem().string();
}

static bool hasLabel(const std::shared_ptr<FunctionNode>& function, const std::string& label) {
    return std::find(function->labels.begin(), function->labels.end(), label) != function->labels.end();
}

void TypeSymbolVisitor::collectDeclarations(ProgramNode &node)
{
    using Scope = std::unordered_map<std::string, std::shared_ptr<FunctionNode>>;

    const std::string mainModule = moduleNameOf(node.moduleName);
    std::string module = mainModule;
    std::unordered_map<std::string, Scope> ownFunctions;
    std::vector<std::pair<std::string, std::shared_ptr<ImportNode>>> imports;

    for (const auto& statement : node.body) {
        if (auto mark = std::dynamic_pointer_cast<ModuleMark>(statement))
            module = moduleNameOf(mark->moduleName);
        else if (auto import = std::dynamic_pointer_cast<ImportNode>(statement))
            imports.push_back({module, import});
        else if (auto structure = std::dynamic_pointer_cast<StructNode>(statement)) {
            // Структуру можно использовать выше её определения
            if (!registry.findStruct(structure->name))
                registry.addStruct(structure->name, std::make_shared<StructNode>(structure->name, structure->body));
        }
        else if (auto function = std::dynamic_pointer_cast<FunctionNode>(statement); function && function->associated.empty()) {
            // У шаблона каждый тип должен выводиться из аргументов: при вызове тип не пишется
            if (!function->typeParameters.empty()) {
                if (hasLabel(function, "@entry") || function->name == "main")
                    LogError("Entry function " + function->name + " cannot be generic", function);
                for (const auto& name : function->typeParameters) {
                    bool mentioned = false;
                    for (const auto& parameter : function->parameters)
                        mentioned = mentioned || typeMentions(parameter.first, name);
                    if (!mentioned)
                        LogError("Type " + name + " of generic function " + function->name + " must appear in its parameters", function);
                }
            }

            ownFunctions[module].emplace(function->name, function);

            // Внутреннее имя, чтобы одинаковые функции разных модулей не мешали друг другу
            if (module != mainModule)
                function->name = module + "::" + function->name;
            declaredFunctions.emplace(function->name, function);
            if (!function->typeParameters.empty()) genericModules[function->name] = module;
        }
    }

    moduleScopes = ownFunctions;

    for (const auto& [importer, import] : imports) {
        this->currentModuleName = importer;
        Scope& scope = moduleScopes[importer];

        const auto addImport = [&](const std::string& name, const std::shared_ptr<FunctionNode>& function) {
            auto existing = scope.find(name);
            if (existing == scope.end()) {
                scope[name] = function;
                return;
            }
            if (existing->second == function) return;

            auto own = ownFunctions[importer].find(name);
            if (own != ownFunctions[importer].end() && hasLabel(own->second, "@overwrite")) return;

            LogError("Imported function '" + name + "' conflicts with another function, import it with an alias or mark your function with @overwrite");
        };

        for (const auto& [path, alias] : import->paths) {
            auto source = ownFunctions.find(path[0]);
            if (source == ownFunctions.end()) continue;

            if (path.size() > 1) {
                auto symbol = source->second.find(path[1]);
                if (symbol == source->second.end()) continue; // Не функция: переменная или структура

                if (hasLabel(symbol->second, "@private"))
                    LogError("Function '" + path[1] + "' is private in module '" + path[0] + "'");

                addImport(alias.empty() ? path[1] : alias, symbol->second);
            }
            else {
                for (const auto& [name, function] : source->second) {
                    if (!hasLabel(function, "@private"))
                        addImport(name, function);
                }
            }
        }
    }
}

void TypeSymbolVisitor::visit(ProgramNode &node)
{
    this->program = std::make_shared<ProgramNode>(node);

    // Сначала собираем объявления, чтобы функцию можно было вызвать выше её определения
    collectDeclarations(node);

    for (const auto& statement : node.body) {
        statement->accept(*this);
    }

    // Кодгену копии шаблонов нужны как обычные функции
    node.body.insert(node.body.end(), genericInstances.begin(), genericInstances.end());
}

void TypeSymbolVisitor::visit(FunctionNode &node)
{
    if (!node.typeParameters.empty()) return; // Проверяются только копии шаблона
    ui::stageDetail(node.name);
    // Проверяем, существует ли функция в реестре(в текущем контексте или глобальном)
    // Если вызов выше определения уже положил сюда эту же функцию, это не повтор
    auto existingFunction = contexts.back().functions.find(node.name);
    if(node.associated.empty() && existingFunction != contexts.back().functions.end() && existingFunction->second != node.shared_from_this()) {
        if(contexts[0].functions.find(node.name) != contexts[0].functions.end()) {
            LogError("Function already defined in global context: " + node.name);
        }
        LogError("Function already defined: " + node.name);
    }

    std::vector<std::string> labels = contexts.back().labels;
    labels.insert(labels.end(), node.labels.begin(), node.labels.end()); // Добавляем метки функции в текущий контекст

    std::unordered_map<std::string, std::shared_ptr<TypeNode>> args = {};

    // Простой тип возврата может быть void, которого нет в реестре типов
    if (std::dynamic_pointer_cast<GenericTypeNode>(node.returnType)) {
        node.returnType->accept(*this);
    }

    // Проходим по параметрам функции
    for (const auto& param : node.parameters) {
        param.first->accept(*this); // Проверяем тип параметра
        // Тут типы параметров не проверяются, так как они могут быть разными и это нормально

        /* TODO: Проверь перекрытие имён
        if(std::find(labels.begin(), labels.end(), "@pure") == labels.end()) { // Если функция не является чистой
            if (contexts.back().variables.find(param.second) != contexts.back().variables.end()) {
                LogError("Parameter already defined: " + param.second);
            }
        }
        */

        if (std::find(node.labels.begin(), node.labels.end(), param.second) != node.labels.end()) {
            LogError("Parameter cannot have the same name as a label: " + param.second);
        }

        if (args.find(param.second) != args.end()) {
            LogError("Parameter already defined: " + param.second, node.shared_from_this());
        }

        // Добавляем параметр в реестр
        args[param.second] = param.first;
    }

    // Если функция не является чистой, то хуярим ей все предыдущие переменные
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> variables = {};
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> functions = {};

    if(std::find(labels.begin(), labels.end(), "@pure") == labels.end())
    {
        // Контекст структуры пустой: метод, как и обычная функция, видит глобальные переменные
        variables = node.associated.empty() ? contexts.back().variables : contexts[0].variables;
        functions = node.associated.empty() ? contexts.back().functions : contexts[0].functions;
    }
    else
    {
        for (const auto& func : contexts.back().functions) {
            if (auto funcNode = std::dynamic_pointer_cast<FunctionNode>(func.second)) {
                if (std::find(funcNode->labels.begin(), funcNode->labels.end(), "@pure") != funcNode->labels.end()) { // Если функция является чистой
                    functions[func.first] = func.second;
                }
            }
        }
    }
    
    // Метод с именем main это обычный метод
    const bool isEntry = node.associated.empty() && (std::find(labels.begin(), labels.end(), "@entry") != labels.end() || node.name == "main");
    if(isEntry)
        if(node.returnType->toString() != "i32" && node.returnType->toString() != "void")
            LogError("Function " + node.name + " must return i32 or void");

    // Аргументов программы пока нет: параметры пришлось бы заполнять нулями, а пустая ссылка это падение
    if(isEntry && !node.parameters.empty())
        LogError("Entry function " + node.name + " cannot have parameters yet", node.shared_from_this());

    // Если не вывилась ошибка, добавляем функцию в реестр
    Context currentFunction = {
        .labels = labels, // Добавляем метки функции
        .variables = variables, // Добавляем переменные текущего контекста
        .functions = functions, // Добавляем функции текущего контекста
        .currentFunctionName = node.name,       // Имя функции 
        .returnType = node.returnType,          // Тип возвращаемого значения
        .returnedValue = false                  // Возвращаемое значение
    };

    // Добавляем параметры функции в текущий контекст
    for (const auto& param : args) {
        auto varNode = std::make_shared<VariableAssignNode>(
            param.first, false, std::dynamic_pointer_cast<TypeNode>(param.second), nullptr);
        
        varNode->inferredType = varNode->type; // <-- инициализация inferredType

        // Добавляем переменную в реестр
        currentFunction.variables[param.first] = varNode;
    }

    // Если функция находится в глобальном контексте, то добавляем ее в глобальный реестр(глобальный контекст - это первый элемент в векторе)
    // Если функция не является методом класса, то добавляем ее в глобальный реестр
    // Метод вызывается только через объект, как функция он не виден
    if(node.associated.empty()) {
        if(contexts.size() == 1)
            contexts[0].functions[node.name] = node.shared_from_this();
        else
            contexts.back().functions[node.name] = node.shared_from_this();

        currentFunction.functions[node.name] = node.shared_from_this(); // Добавляем функцию в текущий контекст
    }
    
    // Добавляем функцию в реестр
    contexts.push_back(currentFunction);

    // Проверяем тело функции
    const bool outerInMethod = insideMethod;
    insideMethod = !node.associated.empty();
    node.body->accept(*this);
    insideMethod = outerInMethod;

    if(contexts.back().returnedValue == false && contexts.back().returnType->toString() != "void") {
        LogError("Function " + node.name + " must return a value of type " + contexts.back().returnType->toString());
    }

    contexts.pop_back(); // Убираем текущий контекст
    
    node.inferredType = node.returnType; // Устанавливаем тип функции
}

void TypeSymbolVisitor::checkCallArguments(std::vector<std::shared_ptr<ASTNode>>& arguments,
                                           const std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>& parameters,
                                           size_t firstParameter, const std::string& name)
{
    for (size_t i = 0; i < arguments.size(); ++i) {
        const auto& parameter = parameters[i + firstParameter].first;
        const std::string paramType = parameter->toString();

        // Литерал сам не знает свой тип, его задаёт параметр
        if (std::dynamic_pointer_cast<BlockNode>(arguments[i])) {
            validateCollectionElements(parameter, arguments[i], false);
            arguments[i]->inferredType = parameter;
            continue;
        }
        if (auto none = std::dynamic_pointer_cast<NoneNode>(arguments[i]);
            none && none->isEmptyLiteral && (paramType.starts_with("array") || paramType.starts_with("map")))
            arguments[i]->inferredType = parameter;

        checkConversion(arguments[i], paramType, name + ", argument " + std::to_string(i + 1) + ": ");
    }
}

void TypeSymbolVisitor::visit(BlockNode &node)
{
    for (const auto& statement : node.statements) {
        statement->accept(*this);

        // Забытая проверка ошибки: write_text(...) без чтения результата молча теряет ошибку
        if (std::dynamic_pointer_cast<CallNode>(statement) || std::dynamic_pointer_cast<AccessExpression>(statement) ||
            std::dynamic_pointer_cast<UnaryOpNode>(statement)) {
            const std::string type = statement->inferredType ? statement->inferredType->toString() : "";
            if (type.starts_with("result"))
                LogError("The result is not used: check it, or write _ ^= ... to ignore it on purpose", statement);
        }
    }
}

static std::string getType(std::shared_ptr<ASTNode> expression, std::string type) {
    if (auto num = std::dynamic_pointer_cast<NumberNode>(expression)) 
        return num->inferredType->toString();
    else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(expression)) 
        if (bin->op == "and" || bin->op == "or" || bin->op == "==" || bin->op == "!=" || bin->op.rfind("icmp_", 0) == 0 || bin->op.rfind("fcmp_", 0) == 0)
            return "i1";
        else
            return type;
    else if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(expression))
        return (unary->op == "neg" || unary->op == "try") ? type : "i1";
    else
        return type;
};

void TypeSymbolVisitor::visit(VariableAssignNode &node)
{
    // Проверяем, существует ли переменная в реестре. _ можно объявлять сколько угодно: так игнорируют result
    if (node.name != "_" && contexts.back().variables.find(node.name) != contexts.back().variables.end()) {
        LogError("Variable already defined: " + node.name);
    }

    // Проверяем тип переменной
    node.type->accept(*this);
    std::string varType = node.type->toString();
    bool isAuto = false;
    bool isStrict = checkLabels("@strict");

    if (varType == "auto")
        if(!isStrict)
            isAuto = true;
        else
            LogError("auto is not allowed in strict mode", node.shared_from_this());

    if (
        varType == "none"
        || varType == "null"
        || varType == "void"
    ) {
        LogError("Type cannot be " + varType, node.shared_from_this());
    }
    
    // Объект это ссылка: без значения в переменной оказалась бы пустая ссылка
    if (auto noValue = std::dynamic_pointer_cast<NoneNode>(node.expression); noValue && !isAuto) {
        const bool isCollection = varType.starts_with("array") || varType.starts_with("map") || varType.starts_with("result");
        if (noValue->isEmptyLiteral && !isCollection)
            LogError("Empty literal cannot initialize variable '" + node.name + "' of type " + varType, node.shared_from_this());

        // final получит значение позже, а читать его до этого нельзя, поэтому пустой ссылки никто не увидит
        if (!noValue->isEmptyLiteral && node.isDeferred) {
            if (contexts.back().currentFunctionName.empty())
                LogError("Global final variable '" + node.name + "' must have a value", node.shared_from_this());
        } else if (!noValue->isEmptyLiteral && node.isConst) {
            LogError("Constant '" + node.name + "' must have a value", node.shared_from_this());
        } else if (!noValue->isEmptyLiteral && (isCollection || registry.findStruct(varType))) {
            LogError("Variable '" + node.name + "' of type " + varType + " must have a value", node.shared_from_this());
        }
    }

    if (isAuto) {
        if (std::dynamic_pointer_cast<NoneNode>(node.expression))
            LogError("Cannot infer type of empty value for variable '" + node.name + "', declare the type explicitly", node.shared_from_this());

        if (auto block = std::dynamic_pointer_cast<BlockNode>(node.expression)) {
            std::shared_ptr<TypeNode> inferredType = infer_collection_type_revised(block);

            if (inferredType->toString() == "auto_empty_collection") {
                bool blockIsMapSyntax = false; 

                if (blockIsMapSyntax) {
                    auto mapType = std::make_shared<GenericTypeNode>("map");
                    mapType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("auto"));
                    mapType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("auto"));
                    node.type = mapType;
                } else {
                    auto arrayType = std::make_shared<GenericTypeNode>("array");
                    arrayType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("auto"));
                    node.type = arrayType;
                }
                LogError("Ambiguous empty collection initializer for 'auto' variable '" + node.name + "'. Resolved based on syntax (assuming array if unclear).", node.shared_from_this());
            } else if (inferredType->toString().rfind("error_", 0) == 0) {
                LogError("Failed to infer type for collection assigned to 'auto' variable '" + node.name + "': " + inferredType->toString(), node.expression);
            }
            else {
                node.type = inferredType;
                node.type->accept(*this); // Выведенный тип подчиняется тем же правилам, что и написанный
            }
            varType = node.type->toString(); // Обновляем varType после вывода
        }
    }
    if (varType.starts_with("array")
        || varType.starts_with("map")
    ) {
#if DEBUG
        ASTDebugger::debug(node.expression);
#endif
        validateCollectionElements(node.type, node.expression, isAuto);
        
    }
    else
    {
#if DEBUG
        IC(node.expression->inferredType);
#endif
        node.expression->accept(*this);
        auto exprType = node.expression->implicitCastTo ? node.expression->implicitCastTo
                                                        : node.expression->inferredType;
        if (exprType && exprType->toString() == "void")
            LogError("Cannot bind void value to a variable", node.expression);
        if (isAuto && exprType && exprType->toString() == "result literal") {
            LogError("Cannot infer the type of " + std::dynamic_pointer_cast<CallNode>(node.expression)->callee + "(), declare it: result<T> " + node.name + " = ...", node.shared_from_this());
        }

        if (std::dynamic_pointer_cast<GenericTypeNode>(exprType)) {
            if (isAuto)
                node.type = exprType;
            else if (exprType->toString() != varType)
                LogError("Type mismatch: expected " + varType + ", got " + exprType->toString(), node.expression);
        }
        else if (!isAuto) {
            if (exprType->toString() != "none")
                checkConversion(node.expression, varType);
        }
        else {
            if (getTypeRank(exprType->toString()) > 0)
                castNumbersInBinaryTree(node.expression, "auto");

            if(node.expression->implicitCastTo)
                node.type = std::make_shared<SimpleTypeNode>(getType(node.expression, node.expression->implicitCastTo->toString()));
            else
                node.type = std::make_shared<SimpleTypeNode>(getType(node.expression, node.expression->inferredType->toString()));
        }
    }

    checkPureAlias(node.expression);

    // Добавляем переменную в реестр
    auto varNode = std::make_shared<VariableAssignNode>(node.name, node.isConst, node.type, node.expression);
    varNode->inferredType = node.type; // Устанавливаем тип переменной
    if (node.type && node.type->toString().find("func") != std::string::npos) {
        LogError("Functions can only be passed as arguments, not stored in variables: " + node.name, node.shared_from_this());
    }
    varNode->isDeferred = node.isDeferred;
    if (node.isDeferred) {
        finalLoopDepth[varNode.get()] = loopDepth;
        deferredFinals.push_back(varNode);
    }

    contexts.back().variables[node.name] = varNode;
}

static bool isCompareOperator(const std::string& op) {
    return op.starts_with("icmp_") || op.starts_with("fcmp_") ||
           op == "and" || op == "or";
}

void TypeSymbolVisitor::visit(ReturnNode &node)
{
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("Return statement outside of function", node.shared_from_this());
    }

    // Проверяем тип возвращаемого значения
    if (node.expression)
    {
        if (auto block = std::dynamic_pointer_cast<BlockNode>(node.expression))
        {
            node.expression->accept(*this);
            validateCollectionElements(contexts.back().returnType, node.expression, false);
        }
        else
        {
            node.expression->accept(*this);
            std::string expectedType = contexts.back().returnType->toString();
            const bool bareReturn = node.expression->inferredType && node.expression->inferredType->toString() == "null";
            if (!(bareReturn && expectedType == "void"))
                checkConversion(node.expression, expectedType, "Return: ");
        }
    }

    checkPureAlias(node.expression);
    contexts.back().returnedValue = true; // Устанавливаем, что функция вернула значение
}

// TODO: Переделать функцию полность потому что она хуйня ебанная
void TypeSymbolVisitor::visit(VariableReassignNode& node) {
    // Проверяем, существует ли переменная в реестре
    if (contexts.back().variables.find(node.name) == contexts.back().variables.end()) {
        LogError("Variable not found: " + node.name, node.shared_from_this());
    }

    std::shared_ptr<VariableAssignNode> varAssign = std::dynamic_pointer_cast<VariableAssignNode>(contexts.back().variables[node.name]);
    std::shared_ptr<TypeNode> varType = varAssign->inferredType;
    std::string varTypeStr = varType->toString();
    node.inferredType = varType; // Кодгену нужен тип, чтобы собрать литерал a = [1, 2]

    if (insideMethod && node.name == "self") {
        LogError("Cannot reassign self", node.shared_from_this());
    }
    if (varAssign->isConst && !varAssign->isDeferred) {
        LogError("Cannot reassign constant variable: " + node.name, node.shared_from_this());
    }
    if (varAssign->isDeferred) {
        if (maybeAssignedFinals.count(varAssign.get()))
            LogError("Final variable '" + node.name + "' is already assigned", node.shared_from_this());
        if (loopDepth > finalLoopDepth[varAssign.get()])
            LogError("Final variable '" + node.name + "' cannot be assigned in a loop", node.shared_from_this());
    }

    std::string expressionType;
    if (varTypeStr.starts_with("array")
        || varTypeStr.starts_with("map")
    ) {
        node.expression->accept(*this);
        checkPureAlias(node.expression);
        validateCollectionElements(varType, node.expression, false);
        auto Block = std::dynamic_pointer_cast<BlockNode>(node.expression);
        auto Generic = std::dynamic_pointer_cast<GenericTypeNode>(varType);

        // a = [] или a = b: validateCollectionElements уже всё проверил
        if (!Block) {
            varAssign->expression = node.expression;
            if (varAssign->isDeferred) {
                assignedFinals.insert(varAssign.get());
                maybeAssignedFinals.insert(varAssign.get());
            }
            return;
        }

        // Элементы уже проверил validateCollectionElements
        expressionType = varTypeStr;
    }
    else
    {
        node.expression->accept(*this);
        checkPureAlias(node.expression);
        checkConversion(node.expression, varTypeStr);
        expressionType = varTypeStr;
    }

    if (expressionType != varTypeStr) 
            LogError("Type mismatch: expected " + varTypeStr + ", got " + expressionType, node.expression);

    // Добавляем переменную в реестр
    varAssign->expression = node.expression;

    if (varAssign->isDeferred) {
        assignedFinals.insert(varAssign.get());
        maybeAssignedFinals.insert(varAssign.get());
    }
}

void TypeSymbolVisitor::visit(IfNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("If statement outside of function", node.shared_from_this());
    }

    node.condition->accept(*this);
    std::string conditionType = node.condition->inferredType->toString();

    if (conditionType != "i1") 
        LogError("If condition must be of type i1, got " + conditionType, node.condition);
    else 
    {
        auto binaryOp = std::dynamic_pointer_cast<BinaryOpNode>(node.condition);
        if(binaryOp && binaryOp->op != "strcmp_eq") 
                castNumbersInBinaryTree(node.condition, "auto");
        else 
                castNumbersInBinaryTree(node.condition, "auto");
    }

    // Ветки идут независимо: присваивание final в одной не видно в другой
    const auto assignedBefore = assignedFinals;
    const auto maybeBefore = maybeAssignedFinals;

    // Создаем новый контекст для блока if
    contexts.push_back(contexts.back());

    contexts.back().currentFunctionName = "if";

    // Проверяем блок if
    node.thenBlock->accept(*this);
    const bool thenReturned = contexts.back().returnedValue;

    // Убираем контекст блока if
    contexts.pop_back();

    const auto assignedInThen = assignedFinals;
    const auto maybeInThen = maybeAssignedFinals;
    assignedFinals = assignedBefore;
    maybeAssignedFinals = maybeBefore;
    bool elseReturnedAll = false;

    // Проверяем блок else
    if (node.elseBlock) {
        // Создаем новый контекст для блока else
        contexts.push_back(contexts.back());

        contexts.back().currentFunctionName = "else";

        // Проверяем блок else
        node.elseBlock->accept(*this);
        const bool elseReturned = contexts.back().returnedValue;
        elseReturnedAll = elseReturned;

        // Убираем контекст блока else
        contexts.pop_back();

        // Если return есть в обеих ветках, то функция точно вернула значение
        if (thenReturned && elseReturned)
            contexts.back().returnedValue = true;
    }

    // После if код продолжается только из веток без return.
    // Здесь сейчас состояние после else или, если его нет, состояние до if
    if (elseReturnedAll && !thenReturned) {
        assignedFinals = assignedInThen;
        maybeAssignedFinals = maybeInThen;
    } else if (!thenReturned) {
        std::set<const ASTNode*> both;
        for (const auto* variable : assignedInThen)
            if (assignedFinals.count(variable)) both.insert(variable);
        assignedFinals = both;
        maybeAssignedFinals.insert(maybeInThen.begin(), maybeInThen.end());
    }
}

void TypeSymbolVisitor::visit(ForNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("For statement outside of function", node.shared_from_this());
    }

    if (contexts.back().variables.find(node.varName) != contexts.back().variables.end()) {
        LogError("Variable already defined: " + node.varName, node.shared_from_this());
    }

    // range это конструкция языка, а не функция: for i in range(n)
    auto range = std::dynamic_pointer_cast<CallNode>(node.iterable);
    if (range && range->callee == "range") {
        if (range->arguments.empty() || range->arguments.size() > 2) {
            LogError("range expects 1 or 2 arguments, got " + std::to_string(range->arguments.size()), node.iterable);
        }

        // Счётчик i32, если значение не требует i64
        std::shared_ptr<TypeNode> limitType;
        std::string counterType = "i32";
        for (const auto& bound : range->arguments) {
            bound->accept(*this);
            limitType = bound->implicitCastTo ? bound->implicitCastTo : bound->inferredType;
            if (!limitType || getTypeRank(limitType->toString()) == 0 || isFloatType(limitType->toString())) {
                LogError("range expects an integer, got " + (limitType ? limitType->toString() : std::string("unknown")), bound);
            }
            if (limitType->toString() == "i64") counterType = "i64";
        }
        for (const auto& bound : range->arguments) {
            checkConversion(bound, counterType);
        }
        node.varType = registry.findType(counterType);
        node.iterable->inferredType = limitType;
    }
    else {
        // Проверяем тип переменной
        node.iterable->accept(*this);
        auto collection = std::dynamic_pointer_cast<GenericTypeNode>(node.iterable->inferredType);
        if (node.iterable->inferredType && node.iterable->inferredType->toString() == "string") {
            node.varType = registry.findType("char"); // for c in s: по байтам строки
        } else {
            if (!collection || (collection->baseName != "array" && collection->baseName != "map")) {
                LogError("for expects an array, map, string or range(n), got " + (node.iterable->inferredType ? node.iterable->inferredType->toString() : std::string("unknown")), node.iterable);
            }

            node.varType = collection->typeParameters[0]; // Переменная цикла это элемент массива
        }
    }

    // Создаем новый контекст для блока for
    contexts.push_back(contexts.back());

    contexts.back().currentFunctionName = "for";

    auto loopVariable = std::make_shared<VariableAssignNode>(node.varName, false, node.varType, nullptr);
    loopVariable->inferredType = node.varType;
    contexts.back().variables[node.varName] = loopVariable;

    // Проверяем блок for
    loopDepth++;
    node.body->accept(*this);
    loopDepth--;

    // Убираем контекст блока for
    contexts.pop_back();
}

void TypeSymbolVisitor::visit(WhileNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("If statement outside of function", node.shared_from_this());
    }

    node.condition->accept(*this);
    std::string conditionType = node.condition->inferredType->toString();

    if (conditionType != "i1") 
        LogError("If condition must be of type i1, got " + conditionType, node.condition);
    else 
    {
        auto binaryOp = std::dynamic_pointer_cast<BinaryOpNode>(node.condition);
        if(binaryOp && binaryOp->op != "strcmp_eq") 
                castNumbersInBinaryTree(node.condition, "auto");
        else 
                castNumbersInBinaryTree(node.condition, "auto");
    }

    // Создаем новый контекст для блока while
    contexts.push_back(contexts.back());

    contexts.back().currentFunctionName = "while";

    // Проверяем блок while
    loopDepth++;
    node.body->accept(*this);
    loopDepth--;

    // Убираем контекст блока while
    contexts.pop_back();
}

void TypeSymbolVisitor::visit(BreakNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("Break statement outside of function", node.shared_from_this());
    }

    bool inLoop = false;
    for (auto it = contexts.rbegin(); it != contexts.rend(); ++it) {
        if (it->currentFunctionName == "for" || it->currentFunctionName == "while") {
            inLoop = true;
            break;
        }
    }

    if (!inLoop) {
        LogError("Break statement outside of loop", node.shared_from_this());
    }

    node.inferredType = std::make_shared<SimpleTypeNode>("void");
}


// TODO: Сделать проверку 
void TypeSymbolVisitor::visit(ContinueNode& node) {
    // Проверяем, существует ли функция в реестре
    if (contexts.back().currentFunctionName.empty()) {
        LogError("Continue statement outside of function", node.shared_from_this());
    }

    bool inLoop = false;
    for (auto it = contexts.rbegin(); it != contexts.rend(); ++it) {
        if (it->currentFunctionName == "for" || it->currentFunctionName == "while") {
            inLoop = true;
            break;
        }
    }

    if (!inLoop) {
        LogError("Continue statement outside of loop", node.shared_from_this());
    }
    node.inferredType = std::make_shared<SimpleTypeNode>("void");
}

void TypeSymbolVisitor::visit(CallNode& node) { 
    // Делаем список типов аргументов
    std::vector<std::shared_ptr<TypeNode>> argTypes;
    for (size_t i = 0; i < node.arguments.size(); ++i) {
        node.arguments[i]->accept(*this);

        if (node.arguments[i]->implicitCastTo)
            argTypes.push_back(node.arguments[i]->implicitCastTo);
        else
            argTypes.push_back(node.arguments[i]->inferredType);
    }

    // keep(x), где keep это параметр типа func. Переменная важнее функции с таким же именем
    if (auto variable = contexts.back().variables.find(node.callee); variable != contexts.back().variables.end()) {
        auto declared = std::dynamic_pointer_cast<VariableAssignNode>(variable->second);
        auto type = declared ? std::dynamic_pointer_cast<GenericTypeNode>(declared->type) : nullptr;
        if (!type || type->baseName != "func") {
            LogError("'" + node.callee + "' is a variable, not a function", node.shared_from_this());
        }

        std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters;
        for (size_t i = 0; i + 1 < type->typeParameters.size(); i++) parameters.push_back({type->typeParameters[i], ""});
        if (node.arguments.size() != parameters.size()) {
            LogError("Function " + node.callee + " expects " + std::to_string(parameters.size()) + " arguments, got " + std::to_string(node.arguments.size()), node.shared_from_this());
        }
        checkCallArguments(node.arguments, parameters, 0, "Function " + node.callee);
        node.functionValueType = type;
        node.inferredType = type->typeParameters.back();
        return;
    }

    // Имя в коде превращаем во внутреннее имя функции: свои и импортированные функции, алиасы
    auto scope = moduleScopes.find(moduleNameOf(currentModuleName));
    if (scope != moduleScopes.end()) {
        auto visible = scope->second.find(node.callee);
        if (visible != scope->second.end())
            node.callee = visible->second->name;
    }

    // copy(x): поверхностная копия объекта. Функция с таким же именем важнее
    if (node.callee == "copy"
        && contexts.back().functions.find(node.callee) == contexts.back().functions.end()
        && declaredFunctions.find(node.callee) == declaredFunctions.end()) {
        if (node.arguments.size() != 1) {
            LogError("copy expects 1 argument, got " + std::to_string(node.arguments.size()), node.shared_from_this());
        }
        if (!argTypes[0]) {
            LogError("copy cannot take a collection literal, assign it to a variable first", node.arguments[0]);
        }
        const std::string copiedType = argTypes[0]->toString();
        if (!copiedType.starts_with("array") && !copiedType.starts_with("map") && !registry.findStruct(copiedType)) {
            LogError("copy expects an array, map or struct, got " + copiedType, node.arguments[0]);
        }
        node.inferredType = argTypes[0];
        return;
    }

    // len(a): длина массива. Функция с таким же именем важнее
    if (node.callee == "len"
        && contexts.back().functions.find(node.callee) == contexts.back().functions.end()
        && declaredFunctions.find(node.callee) == declaredFunctions.end()) {
        if (node.arguments.size() != 1) {
            LogError("len expects 1 argument, got " + std::to_string(node.arguments.size()), node.shared_from_this());
        }
        if (!argTypes[0]) {
            LogError("len cannot take a collection literal, assign it to a variable first", node.arguments[0]);
        }
        if (!argTypes[0]->toString().starts_with("array") && !argTypes[0]->toString().starts_with("map") && argTypes[0]->toString() != "string") {
            LogError("len expects an array, map or string, got " + argTypes[0]->toString(), node.arguments[0]);
        }
        node.inferredType = registry.findType("i32");
        return;
    }

    // cstr(x): значение в строку, тем же способом, что и в склейке "x=" + n. Функция с таким же именем важнее
    if (node.callee == "cstr"
        && contexts.back().functions.find(node.callee) == contexts.back().functions.end()
        && declaredFunctions.find(node.callee) == declaredFunctions.end()) {
        if (node.arguments.size() != 1) {
            LogError("cstr expects 1 argument, got " + std::to_string(node.arguments.size()), node.shared_from_this());
        }
        const std::string argumentType = argTypes[0] ? argTypes[0]->toString() : "a literal";
        if (argumentType == "string") {
            node.inferredType = registry.findType("string");
            return;
        }
        auto isCharArrayLiteral = [&]() -> bool {
            auto blockNode = std::dynamic_pointer_cast<BlockNode>(node.arguments[0]);
            if (!blockNode || blockNode->statements.empty()) return false;
            auto firstStatement = std::dynamic_pointer_cast<NumberNode>(blockNode->statements[0]);
            return firstStatement && firstStatement->isCharLiteral;
        }();

        if (getTypeRank(argumentType) == 0 && argumentType != "char" && !isCharArrayLiteral && argumentType != "array<char>") {
            LogError("cstr expects a number, char or string, got " + argumentType, node.arguments[0]);
        }
        if (std::dynamic_pointer_cast<BinaryOpNode>(node.arguments[0]) || std::dynamic_pointer_cast<UnaryOpNode>(node.arguments[0])) {
            castNumbersInBinaryTree(node.arguments[0], "auto");
        }
        auto converted = std::dynamic_pointer_cast<CallNode>(toStringHandler(node.arguments[0], *this));
        node.callee = converted->callee;
        node.arguments = converted->arguments;
        visit(node);
        return;
    }

    // ok(x) и err(...): их тип станет известен там, куда их положат
    if ((node.callee == "ok" || node.callee == "err")
        && contexts.back().functions.find(node.callee) == contexts.back().functions.end()
        && declaredFunctions.find(node.callee) == declaredFunctions.end()) {
        node.inferredType = std::make_shared<SimpleTypeNode>("result literal");
        return;
    }

    // push(a, x) и pop(a): массив растёт и уменьшается. Функция с таким же именем важнее
    if ((node.callee == "push" || node.callee == "pop")
        && contexts.back().functions.find(node.callee) == contexts.back().functions.end()
        && declaredFunctions.find(node.callee) == declaredFunctions.end()) {
        const size_t expected = node.callee == "push" ? 2 : 1;
        if (node.arguments.size() != expected) {
            LogError(node.callee + " expects " + std::to_string(expected) + " argument" + (expected == 1 ? "" : "s") + ", got " + std::to_string(node.arguments.size()), node.shared_from_this());
        }
        auto array = std::dynamic_pointer_cast<GenericTypeNode>(argTypes[0]);
        if (!array || array->baseName != "array") {
            LogError(node.callee + " expects an array, got " + (argTypes[0] ? argTypes[0]->toString() : std::string("a literal")), node.arguments[0]);
        }

        // Те же правила, что у записи a[i] = x
        if (auto baseId = std::dynamic_pointer_cast<IdentifierNode>(node.arguments[0])) {
            auto it = contexts.back().variables.find(baseId->name);
            auto var = it != contexts.back().variables.end() ? std::dynamic_pointer_cast<VariableAssignNode>(it->second) : nullptr;
            if (var && var->isConst)
                LogError("Cannot modify member of constant variable: " + baseId->name, node.shared_from_this());
            if (var && !var->expression && checkLabels("@pure"))
                LogError("Pure function cannot modify object passed to it: " + baseId->name, node.shared_from_this());
        }

        if (node.callee == "push") {
            std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters = {{argTypes[0], "array"}, {array->typeParameters[0], "value"}};
            checkCallArguments(node.arguments, parameters, 0, "push");
            node.inferredType = std::make_shared<SimpleTypeNode>("void");
        } else {
            node.inferredType = array->typeParameters[0];
        }
        return;
    }

    // has(m, k) и del(m, k). Функция с таким же именем важнее
    if ((node.callee == "has" || node.callee == "del")
        && contexts.back().functions.find(node.callee) == contexts.back().functions.end()
        && declaredFunctions.find(node.callee) == declaredFunctions.end()) {
        if (node.arguments.size() != 2) {
            LogError(node.callee + " expects 2 arguments, got " + std::to_string(node.arguments.size()), node.shared_from_this());
        }
        auto map = std::dynamic_pointer_cast<GenericTypeNode>(argTypes[0]);
        if (!map || map->baseName != "map") {
            LogError(node.callee + " expects a map, got " + (argTypes[0] ? argTypes[0]->toString() : std::string("a literal")), node.arguments[0]);
        }
        checkConversion(node.arguments[1], map->typeParameters[0]->toString(), node.callee + ", key: ");

        if (node.callee == "del") {
            // Те же правила, что у записи m[k] = v
            if (auto baseId = std::dynamic_pointer_cast<IdentifierNode>(node.arguments[0])) {
                auto it = contexts.back().variables.find(baseId->name);
                auto var = it != contexts.back().variables.end() ? std::dynamic_pointer_cast<VariableAssignNode>(it->second) : nullptr;
                if (var && var->isConst)
                    LogError("Cannot modify member of constant variable: " + baseId->name, node.shared_from_this());
                if (var && !var->expression && checkLabels("@pure"))
                    LogError("Pure function cannot modify object passed to it: " + baseId->name, node.shared_from_this());
            }
            node.inferredType = std::make_shared<SimpleTypeNode>("void");
        } else {
            node.inferredType = registry.findType("i1");
        }
        return;
    }

    // Создание структуры: P(). Функция с таким же именем важнее
    if (contexts.back().functions.find(node.callee) == contexts.back().functions.end()
        && declaredFunctions.find(node.callee) == declaredFunctions.end()
        && registry.findStruct(node.callee)) {
        if (!node.arguments.empty()) {
            LogError("Struct " + node.callee + " has no constructor with arguments yet", node.shared_from_this());
        }
        node.inferredType = std::make_shared<SimpleTypeNode>(node.callee);
        return;
    }

    // Вызов шаблона становится вызовом его копии для этих типов
    if (auto declared = declaredFunctions.find(node.callee); declared != declaredFunctions.end()) {
        if (auto generic = std::dynamic_pointer_cast<FunctionNode>(declared->second); generic && !generic->typeParameters.empty())
            node.callee = instantiateGeneric(generic, node, argTypes);
    }

    // Проверяем, существует ли функция в реестре
    if (contexts.back().functions.find(node.callee) == contexts.back().functions.end()) {
        auto declared = declaredFunctions.find(node.callee);

        // Функция объявлена в модуле, в том числе ниже этого вызова
        if (declared != declaredFunctions.end())
            contexts.back().functions[node.callee] = declared->second;

        // Точное совпадение типов аргументов
        // У литерала коллекции типа нет, по типам аргументов такую функцию не найти
        else if (std::find(argTypes.begin(), argTypes.end(), nullptr) == argTypes.end()
                 && registry.findFunction(node.callee, argTypes) != nullptr)
            contexts.back().functions[node.callee] = registry.findFunction(node.callee, argTypes);

        // Поиск функции с совпадением по имени
        else if (registry.findFunction(node.callee) != nullptr) 
            contexts.back().functions[node.callee] = registry.findFunction(node.callee);

        // Функция нихуя не найдена
        else
            LogError("Function not found: " + node.callee, node.shared_from_this());
    }

    // Проверяем типы аргументов
    std::shared_ptr<FunctionNode> func = std::dynamic_pointer_cast<FunctionNode>(contexts.back().functions[node.callee]);

    if (checkLabels("@pure") && std::find(func->labels.begin(), func->labels.end(), "@pure") == func->labels.end())
        LogError("Pure function cannot call impure function: " + node.callee, node.shared_from_this());

    // Проверяем количество аргументов
    if (argTypes.size() != func->parameters.size()) {
        LogError("Function " + node.callee + " expects " + std::to_string(func->parameters.size()) + " arguments, got " + std::to_string(argTypes.size()), node.shared_from_this());
    }

    checkCallArguments(node.arguments, func->parameters, 0, "Function " + node.callee);

    // Чистая функция вызывает то, что ей передали: значит, передать можно только чистую
    if (hasLabel(func, "@pure")) {
        for (const auto& argument : node.arguments) {
            auto reference = std::dynamic_pointer_cast<IdentifierNode>(argument);
            if (reference && reference->referencedFunction && !hasLabel(reference->referencedFunction, "@pure"))
                LogError("Pure function " + node.callee + " can only take @pure functions, " + reference->name + " is not", argument);
        }
    }

    node.inferredType = func->returnType; // Устанавливаем тип функции
}


void TypeSymbolVisitor::visit(AttemptNode& node) {
    if (contexts.back().currentFunctionName.empty())
        LogError("attempt outside of function", node.shared_from_this());
    const auto assignedBefore = assignedFinals, maybeBefore = maybeAssignedFinals;
    contexts.push_back(contexts.back());
    contexts.back().returnedValue = false;
    if (node.errorBody) ++contexts.back().resultHandlerDepth;
    node.body->accept(*this);
    bool allReturned = contexts.back().returnedValue;
    contexts.pop_back();
    const auto assignedBody = assignedFinals, maybeBody = maybeAssignedFinals;
    auto definite = assignedBody;
    auto possible = maybeBody;
    bool hasContinuation = !allReturned;
    auto handler = [&](const std::shared_ptr<BlockNode>& body, const std::string& name, std::shared_ptr<TypeNode> type) {
        if (!body) return;
        assignedFinals = assignedBefore;
        // Failure may occur after any assignment in the body.
        maybeAssignedFinals = maybeBody;
        contexts.push_back(contexts.back());
        contexts.back().returnedValue = false;
        auto binding = std::make_shared<VariableAssignNode>(name, true, type, nullptr);
        binding->inferredType = type;
        contexts.back().variables[name] = binding;
        body->accept(*this);
        const bool returned = contexts.back().returnedValue;
        allReturned = allReturned && returned;
        if (!returned) {
            if (!hasContinuation) definite = assignedFinals;
            else for (auto it = definite.begin(); it != definite.end();) {
                if (!assignedFinals.count(*it)) it = definite.erase(it); else ++it;
            }
            possible.insert(maybeAssignedFinals.begin(), maybeAssignedFinals.end());
            hasContinuation = true;
        }
        contexts.pop_back();
    };
    auto errorType = std::make_shared<GenericTypeNode>("result");
    errorType->typeParameters.push_back(std::make_shared<SimpleTypeNode>("void"));
    handler(node.errorBody, node.errorName, errorType);
    handler(node.panicBody, node.panicName, registry.findType("panic_info"));
    assignedFinals = definite;
    maybeAssignedFinals = possible;
    if (allReturned) contexts.back().returnedValue = true;
}
