#include "headers/ASTVisitors.h"
#include "headers/CodeGenHandlers.h"
#include "../loader/headers/loader.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>           // <<< Убедитесь, что этот include есть
#include <llvm/IR/DerivedTypes.h>   // <<< И этот тоже
#include <llvm/Support/raw_ostream.h> // <<< Добавлено для преобразования типа в строку
#include <string>
#include <string_view>
#include <vector>                   // <<< Добавлено для std::vector
#include "headers/TOMLstd.h" // <<< Добавлено для TOML

// Конструктор
ASTGen::ASTGen(CodeGenContext& context) : context(context), result(nullptr) {}

// Получить результат последнего посещенного узла
llvm::Value* ASTGen::getResult() const { 
    return result; 
}

// Вспомогательная функция для вывода предупреждений
void ASTGen::LogWarning(const std::string& message) {
#if DEBUG
    std::cerr << "Warning: " << message << std::endl;
#endif
}

// Реализации методов visitor для всех узлов AST
void ASTGen::visit(SimpleTypeNode& node) {
    LogWarning("visit не реализован для SimpleTypeNode: " + node.name);
    result = nullptr;
}

void ASTGen::visit(GenericTypeNode& node) {
    LogWarning("visit не реализован для GenericTypeNode: " + node.baseName);
    result = nullptr;
}

// Объявляет функцию в модуле или возвращает уже объявленную
static llvm::Function* declareUserFunction(CodeGenContext& context, FunctionNode& node) {
    if (llvm::Function* existing = context.TheModule->getFunction(node.name)) {
        return existing;
    }

    std::vector<llvm::Type*> paramTypes;
    for (auto& param : node.parameters) {
        paramTypes.push_back(context.getLLVMType(param.first, context.TheContext));
    }

    bool isMain = (node.name == "main" || std::find(node.labels.begin(), node.labels.end(), "@entry") != node.labels.end());
    llvm::Type* returnLLVMType = llvm::Type::getVoidTy(context.TheContext);
    if (node.returnType) {
        returnLLVMType = context.getLLVMType(node.returnType, context.TheContext);
    }
    if (isMain) {
        returnLLVMType = llvm::Type::getInt32Ty(context.TheContext);
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnLLVMType, paramTypes, false);
    return llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.name, context.TheModule.get());
}

// Завершает стартовую функцию: вызывает функцию пользователя и возвращает её код.
// Стартовая функция становится настоящим main, поэтому JIT и собранный файл запускаются одинаково
static void finishStartFunction(CodeGenContext& context, ProgramNode& node, llvm::BasicBlock* startBlock) {
    llvm::Function* start = context.StartFunction;
    context.StartFunction = nullptr;

    llvm::Function* entry = context.TheModule->getFunction(CodeGenContext::findEntryName(node));
    if (!entry) {
        // Запускать нечего, об этом сообщит тот, кто запускает
        start->eraseFromParent();
        return;
    }

    context.Builder.SetInsertPoint(startBlock);
    std::vector<llvm::Value*> arguments;
    for (auto& arg : entry->args()) {
        arguments.push_back(llvm::Constant::getNullValue(arg.getType()));
    }
    context.Builder.CreateRet(context.Builder.CreateCall(entry, arguments));
    context.Builder.ClearInsertionPoint();

    if (llvm::Function* userMain = context.TheModule->getFunction("main")) {
        userMain->setName("__ms_user_main");
    }
    start->setName("main");
}

void ASTGen::visit(ProgramNode& node) {
    LogWarning("visit для ProgramNode: выполняю обработку " + std::to_string(node.body.size()) + " узлов");
    result = nullptr;

    // Раскладка структур нужна раньше всего: P() можно вызвать до объявления структуры
    for (auto& n : node.body) {
        if (auto structure = std::dynamic_pointer_cast<StructNode>(n)) {
            Objects::registerStruct(context, *structure);
        }
    }

    // Сначала объявляем все функции, чтобы их можно было вызывать до определения
    for (auto& n : node.body) {
        if (auto function = std::dynamic_pointer_cast<FunctionNode>(n); function && function->associated.empty()) {
            declareUserFunction(context, *function);
        }
    }

    context.StartFunction = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(context.TheContext), false),
        llvm::Function::ExternalLinkage, "__ms_start", context.TheModule.get()
    );
    llvm::BasicBlock* startBlock = llvm::BasicBlock::Create(context.TheContext, "entry", context.StartFunction);

    context.Builder.SetInsertPoint(startBlock);
    context.Builder.CreateCall(context.getOrDeclareFunction("ms_rt_init",
        llvm::FunctionType::get(llvm::Type::getVoidTy(context.TheContext), false)));
    context.Builder.ClearInsertionPoint();

    for (auto& n : node.body) {
        if (!n) continue;

        // Глобальные переменные инициализируются в стартовой функции в порядке объявления
        if (std::dynamic_pointer_cast<VariableAssignNode>(n)) {
            context.Builder.SetInsertPoint(startBlock);
            n->accept(*this);
            startBlock = context.Builder.GetInsertBlock(); // Выражение могло создать новые блоки
            context.Builder.ClearInsertionPoint();
            continue;
        }

        n->accept(*this);
        // Результат последнего узла становится результатом программы
    }

    finishStartFunction(context, node, startBlock);
#if DEBUG
    context.TheModule->print(llvm::outs(), nullptr);
#endif
}

void ASTGen::visit(FunctionNode& node) {
    LogWarning("visit не полностью реализован для FunctionNode: " + node.name);

    // Сохраняем текущую точку вставки
    llvm::BasicBlock* currentBlock = nullptr;
    llvm::Function* currentFunction = nullptr;
    if (context.Builder.GetInsertBlock()) {
        currentBlock = context.Builder.GetInsertBlock();
        currentFunction = currentBlock->getParent();
    }

    bool isMain = (node.name == "main" || std::find(node.labels.begin(), node.labels.end(), "@entry") != node.labels.end());

    // Создаем минимальную заглушку функции
    llvm::Function* func = declareUserFunction(context, node);
    llvm::Type* returnLLVMType = func->getReturnType();
    auto outerReturnType = context.CurrentReturnType;
    context.CurrentReturnType = node.returnType;


    // Создаем блок входа в функцию
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(
        context.TheContext, "entry", func);
    context.Builder.SetInsertPoint(entryBlock);
    
    // Сохраняем старую таблицу символов и создаем новую
    auto oldNamedValues = context.NamedValues;
    
    // Обрабатываем параметры функции
    unsigned idx = 0;
    for (auto &arg : func->args()) {
        arg.setName(node.parameters[idx++].second);
        // Параметр это обычная переменная: его можно читать и присваивать
        llvm::AllocaInst* paramSlot = context.createEntryAlloca(arg.getType(), arg.getName().str());
        context.Builder.CreateStore(&arg, paramSlot);
        context.NamedValues[arg.getName().str()] = paramSlot;
    }

    // Генерируем тело функции, если оно есть
    if (node.body) {
        node.body->accept(*this);
    }

    // Добавляем return, если его нет
    llvm::BasicBlock* currentInsertionBlock = context.Builder.GetInsertBlock();
    LogWarning("Текущий блок не имеет терминирующей инструкции: " + currentInsertionBlock->getName().str());
    if (!entryBlock->getTerminator()) { // Если текущий блок - это entryBlock
#if DEBUG
        context.TheModule->print(llvm::outs(), nullptr);
#endif
        if (isMain) {
            // main всегда возвращает 0, если пользователь не указал return
            context.Builder.CreateRet(llvm::ConstantInt::get(returnLLVMType, 0));
        } else if (returnLLVMType->isVoidTy()) {
            context.Builder.CreateRetVoid();
        } 
        // TODO: Добавить тут обработки если надо
    }

    // Новая проверка для текущего блока
    if (currentInsertionBlock && currentInsertionBlock != entryBlock && !currentInsertionBlock->getTerminator()) {
        context.Builder.SetInsertPoint(currentInsertionBlock);
        if (isMain) {
            context.Builder.CreateRet(llvm::ConstantInt::get(returnLLVMType, 0));
        } else if (returnLLVMType->isVoidTy()) {
            context.Builder.CreateRetVoid();
        } else {
            // Семантика уже проверила что return есть на всех путях, сюда код не дойдёт
            context.Builder.CreateUnreachable();
        }
    }


    // Восстанавливаем старую таблицу символов
    context.NamedValues = oldNamedValues;
    context.CurrentReturnType = outerReturnType;
    
    result = func;

    context.Builder.ClearInsertionPoint();
}

void ASTGen::visit(StructNode& node) {
    Objects::emitConstructor(context, node);
    result = nullptr;
}

void ASTGen::visit(BlockNode& node) {
    LogWarning("visit для BlockNode: выполняю обработку " + std::to_string(node.statements.size()) + " инструкций");
    result = nullptr;

    // Сохраняем точку вставки до генерации блока
    llvm::BasicBlock* prevBlock = context.Builder.GetInsertBlock();

    bool isInEntryBlock = (prevBlock && prevBlock->getName() == "entry");

    for (auto& stmt : node.statements) {
        if (!stmt) continue;

        stmt->accept(*this);

        // Если это вложенная функция, то чтобы не терялась точка вставки
        if (dynamic_cast<FunctionNode*>(stmt.get()) && prevBlock) {
            context.Builder.SetInsertPoint(prevBlock);
        }
        // Инструкция могла создать новые блоки: if, цикл, проверку деления. Продолжаем с последнего
        else if (prevBlock) {
            prevBlock = context.Builder.GetInsertBlock();
        }
    }

    // Восстанавливаем точку вставки после генерации блока
    if (prevBlock) 
    {
        context.Builder.SetInsertPoint(prevBlock);
    }
}

void ASTGen::visit(VariableAssignNode& node) {
    LogWarning("visit для VariableAssignNode: " + node.name);
    
    // Проверяем, существует ли переменная в таблице символов
    if (context.NamedValues.find(node.name) != context.NamedValues.end()) {
        LogWarning("!!!Переменная " + node.name + " уже существует!!!");
        result = nullptr;
        return;
    }
    
    auto declaredType = node.inferredType ? node.inferredType : node.type;

    // Получаем тип переменной
    llvm::Type* varType;

#if DEBUG
    std::cerr << "Имя " << node.name << std::endl;
#endif

    if (node.inferredType) 
        varType = context.getLLVMType(node.inferredType, context.TheContext);
    else if(node.type)
        varType = context.getLLVMType(node.type, context.TheContext);
    else 
        LogWarning("Не удалось получить тип для переменной " + node.name);
    
#if DEBUG
    std::cerr << "Тип переменной " << node.name << " : " << node.type->toString() << std::endl;
#endif

    LogWarning("Тип переменной поставлен");
    if (!varType) {
        LogWarning("Не удалось получить тип для переменной " + node.name);
        result = nullptr;
        return;
    }

    llvm::BasicBlock* currentBlock = context.Builder.GetInsertBlock();
    bool isGlobalContext = (currentBlock->getParent() == context.StartFunction);
    
    if (isGlobalContext) {
        result = Declarations::handleGlobalVariable(context, node, varType);
        return;
    }
    
    if (Arrays::isLiteral(node.expression)) {
        result = Arrays::handleArrayInitialization(context, node, varType, declaredType);
        return;
    }
    
    // Обычное присваивание (не массив или массив с одним выражением)
    result = Declarations::handleSimpleAssignment(context, node, varType);
}

void ASTGen::visit(ReassignMemberNode& node) {
    auto access = std::dynamic_pointer_cast<AccessExpression>(node.accessExpression);
    if (!access)
        throw std::runtime_error("Codegen Error: присваивание не в элемент массива");

    // Как в Python: сначала значение, потом место, куда оно запишется
    llvm::Value* value = nullptr;
    if (Arrays::isLiteral(node.expression)) {
        value = Arrays::emitLiteral(context, node.expression, node.inferredType);
    } else {
        node.expression->accept(*this);
        value = TypeConversions::loadValueIfPointer(context, getResult());
    }

    llvm::Type* elementType = nullptr;
    llvm::Value* elementPtr = Arrays::emitElementPtr(context, *access, elementType, true);
    context.Builder.CreateStore(TypeConversions::convertValueToType(context, value, elementType), elementPtr);
    result = nullptr;
}

void ASTGen::visit(VariableReassignNode& node) {
    LogWarning("visit не реализован для VariableReassignNode: " + node.name);

    // Тип значения в переменной, а не её адреса
    llvm::Value* variable = context.NamedValues[node.name];
    llvm::Type* varType = nullptr;
    if (auto* local = llvm::dyn_cast<llvm::AllocaInst>(variable))
        varType = local->getAllocatedType();
    else if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(variable))
        varType = global->getValueType();
    if (!varType) {
        LogWarning("Не удалось получить тип для переменной " + node.name);
        result = nullptr;
        return;
    }

    // Генерируем код для присваивания
    result = Declarations::handleSimpleReassignment(context, node, varType);
}

void ASTGen::visit(IfNode& node) {
    LogWarning("visit для IfNode");

    // Генерируем код для условия
    node.condition->accept(*this);
    llvm::Value* condValue = getResult();
    if (!condValue) {
        LogWarning("Ошибка: не удалось вычислить условие");
        result = nullptr;
        return;
    }

    if (!condValue->getType()->isIntegerTy(1)) {
        condValue = context.Builder.CreateICmpNE(
            condValue, 
            llvm::Constant::getNullValue(condValue->getType()),
            "ifcond"
        );
    }

    llvm::Function* function = context.Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(context.TheContext, "then", function);
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(context.TheContext, "ifcont", function);
    llvm::BasicBlock* elseBlock = nullptr;

    if (node.elseBlock) {
        elseBlock = llvm::BasicBlock::Create(context.TheContext, "else", function);
        context.Builder.CreateCondBr(condValue, thenBlock, elseBlock);
    } else {
        context.Builder.CreateCondBr(condValue, thenBlock, mergeBlock);
    }

    // Переменные каждой ветки живут только внутри неё
    auto outerNamedValues = context.NamedValues;

    context.Builder.SetInsertPoint(thenBlock);
    node.thenBlock->accept(*this);
    if (!context.Builder.GetInsertBlock()->getTerminator())
        context.Builder.CreateBr(mergeBlock);
    context.NamedValues = outerNamedValues;

    if (node.elseBlock) {
        context.Builder.SetInsertPoint(elseBlock);
        node.elseBlock->accept(*this);
        if (!context.Builder.GetInsertBlock()->getTerminator())
            context.Builder.CreateBr(mergeBlock);
        context.NamedValues = outerNamedValues;
    }

    context.Builder.SetInsertPoint(mergeBlock);
    result = nullptr;
}

// for x in a: x это копия числа или ссылка на объект, запись в x массив не меняет.
// Массив вычисляется один раз, поэтому a = [...] в теле цикла не влияет на обход
static void emitArrayFor(CodeGenContext& context, ASTGen& codeGen, ForNode& node) {
    llvm::Type* i32Type = llvm::Type::getInt32Ty(context.TheContext);
    llvm::Type* elementType = context.getLLVMType(node.varType, context.TheContext);

    node.iterable->accept(codeGen);
    llvm::Value* array = TypeConversions::loadValueIfPointer(context, codeGen.getResult(), "for.array");
    llvm::Value* length = context.getArrayLength(array);

    llvm::AllocaInst* index = context.createEntryAlloca(i32Type, "for.index");
    llvm::AllocaInst* element = context.createEntryAlloca(elementType, node.varName);
    context.Builder.CreateStore(context.Builder.getInt32(0), index);

    llvm::Function* function = context.Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context.TheContext, "for.cond", function);
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(context.TheContext, "for.body", function);
    llvm::BasicBlock* stepBlock = llvm::BasicBlock::Create(context.TheContext, "for.step", function);
    llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(context.TheContext, "for.end", function);

    context.pushLoopContext(stepBlock, endBlock);
    context.Builder.CreateBr(condBlock);

    context.Builder.SetInsertPoint(condBlock);
    llvm::Value* current = context.Builder.CreateLoad(i32Type, index, "for.current");
    context.Builder.CreateCondBr(context.Builder.CreateICmpSLT(current, length, "for.check"), bodyBlock, endBlock);

    auto outerNamedValues = context.NamedValues;
    context.NamedValues[node.varName] = element;

    // Длина не меняется, индекс всегда в границах: проверка здесь не нужна
    context.Builder.SetInsertPoint(bodyBlock);
    llvm::Value* elementPtr = context.Builder.CreateGEP(elementType, context.getArrayData(array), current, "for.element_ptr");
    context.Builder.CreateStore(context.Builder.CreateLoad(elementType, elementPtr, "for.element"), element);

    node.body->accept(codeGen);
    if (!context.Builder.GetInsertBlock()->getTerminator())
        context.Builder.CreateBr(stepBlock);

    context.NamedValues = outerNamedValues;

    context.Builder.SetInsertPoint(stepBlock);
    llvm::Value* step = context.Builder.CreateLoad(i32Type, index, "for.step.current");
    context.Builder.CreateStore(context.Builder.CreateAdd(step, context.Builder.getInt32(1), "for.next"), index);
    context.Builder.CreateBr(condBlock);

    context.Builder.SetInsertPoint(endBlock);
    context.popLoopContext();
}

void ASTGen::visit(ForNode& node) {
    auto range = std::dynamic_pointer_cast<CallNode>(node.iterable);
    if (!range || range->callee != "range") {
        if (Maps::isMap(node.iterable->inferredType))
            Maps::emitFor(context, *this, node);
        else
            emitArrayFor(context, *this, node);
        result = nullptr;
        return;
    }

    llvm::Type* counterType = context.getLLVMType(node.varType, context.TheContext);

    // Граница считается один раз, до начала цикла
    range->arguments[0]->accept(*this);
    llvm::Value* limit = TypeConversions::loadValueIfPointer(context, getResult(), "limit");
    limit = TypeConversions::convertValueToType(context, limit, counterType, "limit_cast");

    llvm::AllocaInst* counter = context.createEntryAlloca(counterType, node.varName);
    context.Builder.CreateStore(llvm::ConstantInt::get(counterType, 0), counter);

    llvm::Function* function = context.Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context.TheContext, "for.cond", function);
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(context.TheContext, "for.body", function);
    llvm::BasicBlock* stepBlock = llvm::BasicBlock::Create(context.TheContext, "for.step", function);
    llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(context.TheContext, "for.end", function);

    // continue идёт на шаг счётчика, а не на условие
    context.pushLoopContext(stepBlock, endBlock);

    context.Builder.CreateBr(condBlock);

    context.Builder.SetInsertPoint(condBlock);
    llvm::Value* current = context.Builder.CreateLoad(counterType, counter, "for.current");
    context.Builder.CreateCondBr(context.Builder.CreateICmpSLT(current, limit, "for.check"), bodyBlock, endBlock);

    // Переменные тела живут только внутри цикла
    auto outerNamedValues = context.NamedValues;
    context.NamedValues[node.varName] = counter;

    context.Builder.SetInsertPoint(bodyBlock);
    node.body->accept(*this);
    if (!context.Builder.GetInsertBlock()->getTerminator())
        context.Builder.CreateBr(stepBlock);

    context.NamedValues = outerNamedValues;

    context.Builder.SetInsertPoint(stepBlock);
    llvm::Value* step = context.Builder.CreateLoad(counterType, counter, "for.step.current");
    context.Builder.CreateStore(context.Builder.CreateAdd(step, llvm::ConstantInt::get(counterType, 1), "for.next"), counter);
    context.Builder.CreateBr(condBlock);

    context.Builder.SetInsertPoint(endBlock);
    context.popLoopContext();

    result = nullptr;
}

void ASTGen::visit(WhileNode& node) {
    LogWarning("visit для WhileNode");

    llvm::Function* function = context.Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* loopCondBlock = llvm::BasicBlock::Create(context.TheContext, "loop.cond", function);
    llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(context.TheContext, "loop.body", function);
    llvm::BasicBlock* loopEndBlock = llvm::BasicBlock::Create(context.TheContext, "loop.end", function);

    // Сохраняем контекст цикла
    context.pushLoopContext(loopCondBlock, loopEndBlock);

    context.Builder.CreateBr(loopCondBlock);

    context.Builder.SetInsertPoint(loopCondBlock);
    node.condition->accept(*this);
    llvm::Value* condValue = getResult();
    if (!condValue) {
        LogWarning("Ошибка: не удалось вычислить условие для WhileNode");
        if (!context.Builder.GetInsertBlock()->getTerminator()) {
            context.Builder.CreateBr(loopEndBlock);
        }
        context.popLoopContext(); // Не забываем очистить стек при ошибке
        result = nullptr;
        return;
    }

    if (!condValue->getType()->isIntegerTy(1)) {
        condValue = context.Builder.CreateICmpNE(
            condValue,
            llvm::Constant::getNullValue(condValue->getType()),
            "whilecond.tobool"
        );
    }
    context.Builder.CreateCondBr(condValue, loopBodyBlock, loopEndBlock);

    // Переменные тела живут только внутри цикла
    auto outerNamedValues = context.NamedValues;

    context.Builder.SetInsertPoint(loopBodyBlock);
    if (node.body) {
        node.body->accept(*this);
    }
    context.NamedValues = outerNamedValues;
    
    if (!context.Builder.GetInsertBlock()->getTerminator()) {
        context.Builder.CreateBr(loopCondBlock);
    }

    context.Builder.SetInsertPoint(loopEndBlock);

    // Восстанавливаем/очищаем контекст цикла
    context.popLoopContext();

    result = nullptr;
}

void ASTGen::visit(ReturnNode& node) {
    result = Statements::handleReturnStatement(context, node);
}

void ASTGen::visit(CallNode& node) {
    LogWarning("visit не реализован для CallNode: " + node.callee);
    // Сначала в модуле ищем хуйню
    llvm::Function* calleeFunc = context.TheModule->getFunction(node.callee);

    // Встроенная len(a). Функция с таким же именем важнее, семантика решила так же
    if (!calleeFunc && node.callee == "len") {
        node.arguments[0]->accept(*this);
        llvm::Value* collection = TypeConversions::loadValueIfPointer(context, getResult());
        result = Maps::isMap(node.arguments[0]->inferredType) ? Maps::emitLen(context, collection) : context.getArrayLength(collection);
        return;
    }

    // Встроенные has(m, k) и del(m, k)
    if (!calleeFunc && (node.callee == "has" || node.callee == "del")) {
        node.arguments[0]->accept(*this);
        llvm::Value* map = TypeConversions::loadValueIfPointer(context, getResult());
        llvm::Value* key = Maps::emitKey(context, node.arguments[1], node.arguments[0]->inferredType);
        if (node.callee == "has") {
            result = Maps::emitHas(context, map, key);
        } else {
            Maps::emitDel(context, map, key, node.line);
            result = nullptr;
        }
        return;
    }

    // copy(x) и P(). Функция с таким же именем важнее, семантика решила так же
    if (!calleeFunc && node.callee == "copy") {
        node.arguments[0]->accept(*this);
        result = Objects::emitCopy(context, TypeConversions::loadValueIfPointer(context, getResult()), node.inferredType);
        return;
    }
    if (!calleeFunc && context.Structs.count(node.callee)) {
        result = Objects::emitNew(context, node.callee);
        return;
    }

    // Если не нашли пробуем объявить через TOML
    if (!calleeFunc) {
        std::string TOML_path = loader::findTomlPath();
        if (TOML_path.empty()) {
            LogWarning("Не удалось найти TOML файл для функции " + node.callee);
            result = nullptr;
            return;
        }
        
        calleeFunc = declareFunctionFromTOML(node.callee, context.TheModule.get(), context.TheContext, TOML_path);
    }

    if (!calleeFunc) {
        throw std::runtime_error("Codegen Error: неизвестная функция: " + node.callee);
    }

    if (calleeFunc->arg_size() != node.arguments.size()) {
        LogWarning("Неверное количество аргументов для функции " + node.callee);
        result = nullptr;
        return;
    }

    std::vector<llvm::Value*> argsV;
    for (unsigned i = 0, e = node.arguments.size(); i != e; ++i) {
        if (!node.arguments[i]) {
            LogWarning("Пустой узел аргумента для функции " + node.callee);
            result = nullptr;
            return;
        }
        llvm::Value* argVal = nullptr;
        if (Arrays::isLiteral(node.arguments[i])) {
            argVal = Arrays::emitLiteral(context, node.arguments[i], node.arguments[i]->inferredType);
        } else {
            node.arguments[i]->accept(*this);
            argVal = getResult();
        }
        if (!argVal) {
            LogWarning("Не удалось сгенерировать аргумент " + std::to_string(i) + " для функции " + node.callee);
            result = nullptr;
            return;
        }

        // Сначала значение переменной, потом приведение: и то, что выбрала семантика, и к типу параметра
        argVal = TypeConversions::loadValueIfPointer(context, argVal, "arg");
        argVal = TypeConversions::applyImplicitCast(context, argVal, node.arguments[i]->implicitCastTo);
        argVal = TypeConversions::convertValueToType(context, argVal, calleeFunc->getFunctionType()->getParamType(i));

        argsV.push_back(argVal);
    }

    llvm::Type* retType = calleeFunc->getReturnType();
    if (retType->isVoidTy()) {
        result = context.Builder.CreateCall(calleeFunc, argsV);
    } else {
        result = context.Builder.CreateCall(calleeFunc, argsV, "calltmp");
    }
}

void ASTGen::visit(ModuleMark &node)
{
    LogWarning("visit для ModuleMark: " + node.moduleName);
    this->currentModuleName = node.moduleName;
}

void ASTGen::visit(BinaryOpNode& node) {
    LogWarning("Обработка BinaryOpNode: " + node.op);

    if (!node.left) {
        LogWarning("Ошибка: левый операнд равен nullptr");
        result = nullptr;
        return;
    }
    node.left->accept(*this);
    llvm::Value* left = getResult();
    
    if (!left) {
        LogWarning("Ошибка: не удалось получить значение левого операнда");
        result = nullptr;
        return;
    }
  
    if (!node.right) {
        LogWarning("Ошибка: правый операнд равен nullptr");
        result = nullptr;
        return;
    }
    node.right->accept(*this);
    llvm::Value* right = getResult();
    
    if (!right) {
        LogWarning("Ошибка: не удалось получить значение правого операнда");
        result = nullptr;
        return;
    }
    
    result = Expressions::handleBinaryOperation(context, node, left, right);
}

void ASTGen::visit(UnaryOpNode& node) {
    LogWarning("Обработка UnaryOpNode: " + node.op);

    if (!node.operand) {
        LogWarning("Ошибка: операнд равен nullptr");
        result = nullptr;
        return;
    }
    
    node.operand->accept(*this);
    llvm::Value* operand = getResult();
    
    if (!operand) {
        LogWarning("Ошибка: не удалось получить значение операнда");
        result = nullptr;
        return;
    }
    
    result = Expressions::handleUnaryOperation(context, node, operand);
}

void ASTGen::visit(IdentifierNode& node) {
    LogWarning("visit для IdentifierNode: " + node.name);

    // Базовая заглушка: ищем переменную в таблице символов
    if (context.NamedValues.find(node.name) != context.NamedValues.end()) {
        result = context.NamedValues[node.name];
        LogWarning("Найдена переменная " + node.name);
        return;
    }

    result = nullptr;
}

void ASTGen::visit(NumberNode& node) {
    LogWarning("visit для NumberNode: " + std::to_string(node.value));

    llvm::Type* targetLLVMType = nullptr;

    if (node.implicitCastTo)
    {
        targetLLVMType = context.getLLVMType(node.implicitCastTo, context.TheContext); 
        if (!targetLLVMType) {
             LogWarning("Не удалось получить целевой тип LLVM из implicitCastTo. Используется i32 по умолчанию.");
             targetLLVMType = llvm::Type::getInt32Ty(context.TheContext);
        }
    }
    else
    {
        targetLLVMType = context.getLLVMType(node.inferredType, context.TheContext);
        LogWarning("Нет типа для неявного приведения. Используется i32 по умолчанию.");
    }

    if (targetLLVMType && targetLLVMType->isIntegerTy()) 
        result = llvm::ConstantInt::get(targetLLVMType, node.value, true);
    else if (targetLLVMType && targetLLVMType->isFloatingPointTy()) 
    {
        LogWarning("Неявное приведение NumberNode к типу с плавающей точкой. Создается ConstantFP.");
        double floatValue = static_cast<double>(node.value);
        result = llvm::ConstantFP::get(llvm::Type::getFloatTy(context.TheContext), llvm::APFloat(static_cast<float>(floatValue)));
    }
    else {
        // Обработка других типов или если targetLLVMType все еще nullptr
        LogWarning("Неподдерживаемый или неизвестный целевой тип для NumberNode. Используется i32 по умолчанию.");
        targetLLVMType = llvm::Type::getInt32Ty(context.TheContext);
        result = llvm::ConstantInt::get(targetLLVMType, node.value, true);
    }
}

void ASTGen::visit(FloatNumberNode& node) {
    LogWarning("visit для FloatNumberNode: " + std::to_string(node.value));
    result = llvm::ConstantFP::get(context.TheContext, llvm::APFloat(node.value));
}

void ASTGen::visit(StringNode& node) {
    LogWarning("visit для StringNode: \"" + node.value + "\"");
    result = context.Builder.CreateGlobalString(node.value, "str");
}

void ASTGen::visit(NullNode& node) {
    LogWarning("visit для NullNode");
    result = context.getNullValue();
}

void ASTGen::visit(NoneNode& node) {
    LogWarning("visit для NoneNode");
    result = context.getNoneValue();
}

void ASTGen::visit(KeyValueNode& node) {
    LogWarning("visit не реализован для KeyValueNode");
    result = nullptr;
}

void ASTGen::visit(BreakNode& node) {
    LogWarning("visit для BreakNode");
    llvm::BasicBlock* loopEnd = context.getCurrentLoopEndBlock();
    if (loopEnd) {
        if (!context.Builder.GetInsertBlock()->getTerminator()) {
            context.Builder.CreateBr(loopEnd);
        } else {
            LogWarning("Break statement in a block that is already terminated.");
        }
    }
    result = nullptr; // break не производит значения
}

void ASTGen::visit(ContinueNode& node) {
    LogWarning("visit для ContinueNode");
    llvm::BasicBlock* loopCond = context.getCurrentLoopCondBlock();
    if (loopCond) {
        if (!context.Builder.GetInsertBlock()->getTerminator()) {
            context.Builder.CreateBr(loopCond);
        } else {
            LogWarning("Continue statement in a block that is already terminated.");
        }
    }
    result = nullptr; // continue не производит значения
}

void ASTGen::visit(AccessExpression& node) {
    llvm::Type* elementType = nullptr;
    llvm::Value* elementPtr = Arrays::emitElementPtr(context, node, elementType);
    result = context.Builder.CreateLoad(elementType, elementPtr, "element");
}

void ASTGen::visit(ImportNode &node)
{
    /*
    Они уже убраны к хуям собачим из AST перед этапом семантики
            Смысла реализовывать здесь что то нет
    */
}

void ASTGen::visit(LambdaNode &node)
{
    LogWarning("visit не реализован для LambdaNode");
    result = nullptr;
}