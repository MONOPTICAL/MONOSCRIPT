#include "headers/CodeGenContext.h"
#include "../errors/headers/ErrorEngine.h"
#include <iostream>

// Получает или объявляет функцию в модуле, если она ещё не существует
llvm::Function* CodeGenContext::getOrDeclareFunction(const std::string& name, llvm::FunctionType* type) {
    llvm::Function* func = TheModule->getFunction(name); // Получаем функцию с модуля
    if (!func) // Проверяем существует ли уже эта самая функция
    { // Если функция не найдена в модуле, создаём её 
        const bool checked = name == "ms_panic_index" || name == "ms_panic_result" ||
            name == "ms_str_at" || name == "ms_map_get" || name == "ms_map_del" || name == "ms_map_next";
            
        // Если функция является одной из проверяемых, добавляем дополнительный аргумент для контекста паники
        if (checked) {
            // Копируем все аргументы существующего типа функции в новый вектор
            std::vector<llvm::Type*> args(type->params().begin(), type->params().end());
            // Втюхиваем дополнительный аргумент для контекста паники
            args.push_back(Builder.getPtrTy());
            // Создаём новый тип функции с дополнительным аргументом
            type = llvm::FunctionType::get(type->getReturnType(), args, false);
        }
        // Создаём функцию в модуле
        func = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, TheModule.get());
        // Если функция была успешно создана, добавляем атрибут для проверяемых функций
        if (checked) func->addFnAttr("ms.panic_context");
    }
    return func;
}

llvm::Type* CodeGenContext::getLLVMType(std::shared_ptr<TypeNode> typeNode, llvm::LLVMContext& ctx) {
    // TODO: Реализовать хуйню для сложных типов как array<array<array<i16>>>

    if (auto simpleType = std::dynamic_pointer_cast<SimpleTypeNode>(typeNode)) {
        if (simpleType->name == "i1") return llvm::Type::getInt1Ty(ctx); // bool
        if (simpleType->name == "i8") return llvm::Type::getInt8Ty(ctx); // 8-битное целое
        if (simpleType->name == "i16") return llvm::Type::getInt16Ty(ctx); // 16-битное целое
        if (simpleType->name == "i32") return llvm::Type::getInt32Ty(ctx); // 32-битное целое
        if (simpleType->name == "i64") return llvm::Type::getInt64Ty(ctx); // 64-битное целое
        if (simpleType->name == "float") return llvm::Type::getFloatTy(ctx); // 32-битное число с плавающей точкой
        if (simpleType->name == "f64") return llvm::Type::getDoubleTy(ctx); // 64-битное число с плавающей точкой
        if (simpleType->name == "char") return llvm::Type::getInt8Ty(ctx); // символ
        if (simpleType->name == "string") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // Строки как char*
        if (simpleType->name == "void") return llvm::Type::getVoidTy(ctx); // пустой тип
        if (simpleType->name == "null") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // null как указатель на i8
        if (simpleType->name == "auto") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // auto как указатель на i8
    }
    else if (auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(typeNode)) {
        // Коллекции это объекты в куче, переменная хранит на них ссылку
        return llvm::PointerType::getUnqual(ctx);
    }
    
    return llvm::PointerType::getUnqual(ctx); // По умолчанию возвращаем указатель на i8 для неизвестных типов
}

// Получает тип по AST-узлу, используемый для инференса типов переменных и выражений
std::shared_ptr<TypeNode> CodeGenContext::getTypeByASTNode(std::shared_ptr<ASTNode> node) {
    if (auto typeNode = std::dynamic_pointer_cast<TypeNode>(node)) {
        return typeNode;
    }
    else if (auto numberNode = std::dynamic_pointer_cast<NumberNode>(node)) {
        return std::make_shared<SimpleTypeNode>(numberNode->type->toString());
    }
    else if (auto stringNode = std::dynamic_pointer_cast<StringNode>(node)) {
        return std::make_shared<SimpleTypeNode>("string");
    }
    else if (auto floatNode = std::dynamic_pointer_cast<FloatNumberNode>(node)) {
        return std::make_shared<SimpleTypeNode>("float");
    }
    else if (auto nullNode = std::dynamic_pointer_cast<NullNode>(node)) {
        return std::make_shared<SimpleTypeNode>("null");
    }
    throw std::runtime_error("Unknown ASTNode type for type inference : " + std::to_string(node->line) + ":" + std::to_string(node->column));
}

llvm::StructType* CodeGenContext::getArrayStruct() {
    if (llvm::StructType* existing = llvm::StructType::getTypeByName(TheContext, "ms_array"))
        return existing;

    return llvm::StructType::create(
        TheContext,
        {
            llvm::PointerType::get(TheContext, 0), // data
            llvm::Type::getInt32Ty(TheContext),    // length
            llvm::Type::getInt32Ty(TheContext)     // capacity
        },
        "ms_array"
    );
}

llvm::StructType* CodeGenContext::getResultStruct(const std::shared_ptr<TypeNode>& resultType) {
    std::vector<llvm::Type*> fields = {Builder.getInt8Ty(), Builder.getInt32Ty(), Builder.getPtrTy()};
    auto generic = std::dynamic_pointer_cast<GenericTypeNode>(resultType);
    if (generic && !generic->typeParameters.empty() && generic->typeParameters[0]->toString() != "void")
        fields.push_back(getLLVMType(generic->typeParameters[0], TheContext));
    return llvm::StructType::get(TheContext, fields);
}

llvm::Value* CodeGenContext::allocateObject(llvm::Value* bytes, bool hasReferences, const std::string& name) {
    llvm::FunctionType* allocType = llvm::FunctionType::get(
        llvm::PointerType::get(TheContext, 0), {llvm::Type::getInt64Ty(TheContext)}, false);
    return Builder.CreateCall(getOrDeclareFunction(hasReferences ? "GC_malloc" : "GC_malloc_atomic", allocType), {bytes}, name);
}

llvm::Value* CodeGenContext::createArray(llvm::Type* elementType, llvm::Value* size) {
    llvm::StructType* arrayStruct = getArrayStruct(); // Получаем LLVM'овское описание структуры массива
                                                      // array = {ptr, i32, i32} - data, length, capacity
    const llvm::DataLayout& layout = TheModule->getDataLayout();
    // Это справочник для llvm как ложаться типы в памяти, о их размерах и выравнивании
    // Он его подбирает под архитектуру и ABI платформы на которой запущен компилятор так что тут н3хуй надумывать особо

    // В заголовке лежит ссылка на {ptr, i32, i32} - data, length, capacity
    llvm::Value* header = allocateObject(Builder.getInt64(layout.getTypeAllocSize(arrayStruct)), true, "array");
    // TODO: layout не подстраивается под систему, а просто ебашит дефотлный x86-64, нужно учитывать архитектуру целевой платформы

    llvm::Value* bytes = Builder.CreateMul( // А вот тута мы вычисляем сколько байтов надо для data основываясь на size
        Builder.CreateZExt(size, Builder.getInt64Ty()), // 3башим размер в 64-битное целое чтобы умножать его на кличество элементов
        Builder.getInt64(layout.getTypeAllocSize(elementType)), // Получаем размер одного элемента в байтах
        "array_bytes" // Имя для temp в котором будет храниться количество байтов для data
    );
    llvm::Value* data = allocateObject(bytes, elementType->isPointerTy(), "array_data"); // аллоцируем память для data массива
                                                                                         // bytes - количество байтов для data массива
                                                                                         // elementType->isPointerTy() - говорит, содержит ли элемент указатель, чтобы GC мог правильно обрабатывать память
                                                                                         // "array_data" - имя для temp в котором будет храниться указатель на data массива
    Builder.CreateStore(data, Builder.CreateStructGEP(arrayStruct, header, 0)); // Сохраняем указатель на data в структуре массива
    Builder.CreateStore(size, Builder.CreateStructGEP(arrayStruct, header, 1)); // Сохраняем длину массива
    Builder.CreateStore(size, Builder.CreateStructGEP(arrayStruct, header, 2)); // Сохраняем capacity массива
    return header;
}
// Пиздит с {ptr data ,i32 size ,i32 capacity} data
llvm::Value* CodeGenContext::getArrayData(llvm::Value* array) {
    llvm::Value* field = Builder.CreateStructGEP(getArrayStruct(), array, 0);
    return Builder.CreateLoad(llvm::PointerType::get(TheContext, 0), field, "array_data");
}

// Пиздит с {ptr data ,i32 size ,i32 capacity} size
llvm::Value* CodeGenContext::getArrayLength(llvm::Value* array) {
    llvm::Value* field = Builder.CreateStructGEP(getArrayStruct(), array, 1);
    return Builder.CreateLoad(llvm::Type::getInt32Ty(TheContext), field, "array_len");
}

llvm::Value* CodeGenContext::getArrayElementPtr(llvm::Value* array, llvm::Type* elementType, llvm::Value* index, int line) {
    llvm::Type* i64Type = llvm::Type::getInt64Ty(TheContext); // Получаем тип i64, чтобы не вызывать llvm::Type::getInt64Ty каждый раз
    llvm::Type* i32Type = llvm::Type::getInt32Ty(TheContext); // Тут одно и тоже только теперь для i32

    // Если индекс у нас i1(0 и 1 в семантике получают i1, почему? потому что проект мой и я делаю чё хочу блять)
    // То расширяем его в i64, чтобы корректно сравнивать с длиной массива, а если нет то всё равно расширяем в i64 просто методы разные для этих операций.
    llvm::Value* wideIndex = index->getType()->isIntegerTy(1)
        ? Builder.CreateZExt(index, i64Type, "index")
        : Builder.CreateSExt(index, i64Type, "index");

    // Получаем длину массива, благо это не проблема, просто берём size из {ptr, i32, i32}(data,size,capacity)
    llvm::Value* length = getArrayLength(array); // Метод getArrayLength сверху этого если чё

    llvm::Function* function = Builder.GetInsertBlock()->getParent(); // Получаем текущую функцию, чтобы в правильный блок поставить а не хуй пойми куда

    // А теперь наш любимый error-handling
    // Как ещё дохуя других действий мы можем получить по еблу от компилятора, например тот же OUT_OF_RANGE
    // Так чтобы такие вещи ловить мы делаем два блока, bounds.fail и bounds.ok, и в зависимости от условия переходим в один из них
    llvm::BasicBlock* failBlock = llvm::BasicBlock::Create(TheContext, "bounds.fail", function);
    llvm::BasicBlock* okBlock = llvm::BasicBlock::Create(TheContext, "bounds.ok", function);

    Builder.CreateCondBr(Builder.CreateICmpUGE(wideIndex, Builder.CreateZExt(length, i64Type)), failBlock, okBlock);
    // Теперь чтобы понять пошло ли у нас всё по пизде мы сравниваем индекс с длиной массива оператором чтобы понять является ли индекс больше или равен размеру
    // Если индекс больше или равен длине массива, то переходим в failBlock, а если нет то тогда заебись и мы переходим в okBlock
    // okBlock теперь является блоком в котором мы продолжаем все дальнейшие действия в функции до пока не перейдём в другой блок или не завершится функция

    // Ну а теперь надо этот failBlock чем-то заполнить, в нём особо нихуя и нет кроме вызова ms_panic_index который прописан в rt.d
    Builder.SetInsertPoint(failBlock);

    // Создаём или получаем объявление функции ms_panic_index(long index, int length, int line, MsPanicContext* panic) 
    llvm::Function* panic = getOrDeclareFunction("ms_panic_index",
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), {i64Type, i32Type, i32Type}, false));

    // Вызываем чисто эту самую функцию 
    Builder.CreateCall(panic, {wideIndex, length, Builder.getInt32(ErrorEngine::getInstance().displayLine(line)), PanicContext});
    propagatePanic(); // И также как и для любого другого call'а, мы должны обработать панику после вызова функции
    
    // Потом переходим в okBlock
    Builder.SetInsertPoint(okBlock); 
    return Builder.CreateGEP(elementType, getArrayData(array), wideIndex, "element_ptr");
}

// Вызывает панику
void CodeGenContext::emitPanic(const std::string& message, int line) {
    // Создаём или получаем объявление функции void ms_raise(const char* message, int line, MsPanicContext* panic)
    auto raise = getOrDeclareFunction("ms_raise", llvm::FunctionType::get(
        Builder.getVoidTy(), // void
        {
            Builder.getPtrTy(), // const char* message
            Builder.getInt32Ty(), // int line
            Builder.getPtrTy()}, // MsPanicContext* panic
            false // а вот это означает, что функция не принимает переменное число аргументов
        ));

    // Вызываем функцию ms_raise с переданными аргументами
    Builder.CreateCall(raise, {createString(message),
        Builder.getInt32(ErrorEngine::getInstance().displayLine(line)), PanicContext});
    propagatePanic();
}

// Возвращает тип структуры паники
llvm::StructType* CodeGenContext::getPanicStruct() {
    return llvm::StructType::get(TheContext, {Builder.getInt8Ty(), Builder.getInt32Ty(), Builder.getPtrTy(), Builder.getPtrTy()});
}

// Наполняет вертку panic.propagate которая обрабатывает что делать с истерикой
void CodeGenContext::propagatePanic() {
    if (!PanicHandlers.empty()) { // attempt создаёт panic handler и если он есть значит вызов был внутри attempt
        Builder.CreateBr(PanicHandlers.back()); // Переходим в panic info
        return;
    }
    // Если обработчика нет, значит паника произошла вне attempt, нужно завершить функцию
    auto function = Builder.GetInsertBlock()->getParent();
    if (function->hasFnAttribute("ms.panic_context")) {
        // Каждая функция в языке имеет атрибут "ms.panic_context"
        // Если тип возвращаемого значения void, просто возвращаемся, иначе возвращаем null значение
        if (function->getReturnType()->isVoidTy()) Builder.CreateRetVoid(); 
        else Builder.CreateRet(llvm::Constant::getNullValue(function->getReturnType()));
        return;
    }

    // А если мы оказываемся тут значит мы в @__ms_user_main и бежать особо некуда, так что просто вызываем функцию ms_panic и крашаем всё к хуям собачийм
    auto layout = getPanicStruct(); // Получаем структуру паники
    auto message = Builder.CreateLoad(Builder.getPtrTy(), Builder.CreateStructGEP(layout, PanicContext, 2)); // Берём сообщение из структуры паники
    auto line = Builder.CreateLoad(Builder.getInt32Ty(), Builder.CreateStructGEP(layout, PanicContext, 1)); // Берём номер строки из структуры паники
    auto fatal = getOrDeclareFunction("ms_panic", llvm::FunctionType::get(Builder.getVoidTy(),
        {Builder.getPtrTy(), Builder.getInt32Ty()}, false)); // Вызываем функцию ms_panic с сообщением и номером строки
    fatal->setDoesNotReturn(); // Указываем, что функция не возвращает управление
    Builder.CreateCall(fatal, {message, line}); // Вызываем функцию ms_panic с сообщением и номером строки
    Builder.CreateUnreachable(); // Указываем, что этот блок кода недостижим после вызова ms_panic
}

// Проверяем, произошла ли паника, и если да, обрабатываем её
void CodeGenContext::checkPanic() {
    auto function = Builder.GetInsertBlock()->getParent(); // Получаем текущую функцию
    auto fail = llvm::BasicBlock::Create(TheContext, "panic.propagate", function); // Блок для обработки паники
    auto ok = llvm::BasicBlock::Create(TheContext, "panic.clear", function); // Блок для продолжения выполнения, если паники нет
    auto flag = Builder.CreateLoad(Builder.getInt8Ty(), Builder.CreateStructGEP(getPanicStruct(), PanicContext, 0)); // Загружаем флаг паники
    Builder.CreateCondBr(Builder.CreateICmpNE(flag, Builder.getInt8(0)), fail, ok); // Если флаг не равен 0, переходим в блок fail, иначе в ok
    Builder.SetInsertPoint(fail);
    auto lineSlot = Builder.CreateStructGEP(getPanicStruct(), PanicContext, 1); // Получаем указатель на поле с номером строки
    auto line = Builder.CreateLoad(Builder.getInt32Ty(), lineSlot); // Загружаем номер строки
    Builder.CreateStore(Builder.CreateSelect(Builder.CreateICmpEQ(line, Builder.getInt32(0)),
        Builder.getInt32(ErrorEngine::getInstance().displayLine(CurrentLine)), line), lineSlot); // Если номер строки равен 0, подставляем текущую строку, иначе оставляем как есть
    propagatePanic();
    Builder.SetInsertPoint(ok); // Продолажаем читать программу в блоке ok, если паники нетx`
}

// 
llvm::Value* CodeGenContext::emitCall(llvm::Function* function, std::vector<llvm::Value*> arguments, const std::string& name) {
    const bool checked = function->hasFnAttribute("ms.panic_context"); // Если в функции есть ms.panic_context {bool pending, int line, char* message, ptr stackLimit}
    if (checked) arguments.push_back(PanicContext); // То добавляем контекст паники в аргументы функции
    auto value = Builder.CreateCall(function, arguments, function->getReturnType()->isVoidTy() ? "" : name); // Зовём функцию с аргументами
    if (checked) checkPanic(); // Если функция использует контекст паники, проверяем его после вызова функции
    return value;
}

// Every func<> value uses the language calling convention, including native functions.
llvm::Function* CodeGenContext::functionAdapter(llvm::Function* function) {
    if (function->hasFnAttribute("ms.panic_context")) return function;
    const std::string name = "__ms_adapter_" + function->getName().str();
    if (auto existing = TheModule->getFunction(name)) return existing;
    std::vector<llvm::Type*> types(function->getFunctionType()->params().begin(), function->getFunctionType()->params().end());
    types.push_back(Builder.getPtrTy());
    auto adapter = llvm::Function::Create(llvm::FunctionType::get(function->getReturnType(), types, false),
        llvm::Function::PrivateLinkage, name, TheModule.get());
    adapter->addFnAttr("ms.panic_context");
    llvm::IRBuilder<> builder(llvm::BasicBlock::Create(TheContext, "entry", adapter));
    std::vector<llvm::Value*> args;
    for (unsigned i = 0; i < function->arg_size(); ++i) args.push_back(adapter->getArg(i));
    auto value = builder.CreateCall(function, args);
    if (function->getReturnType()->isVoidTy()) builder.CreateRetVoid(); else builder.CreateRet(value);
    return adapter;
}

std::string CodeGenContext::findEntryName(const ProgramNode& program) {
    std::string entryName = "main";
    for (const auto& node : program.body) {
        if (auto func = std::dynamic_pointer_cast<FunctionNode>(node)) {
            for (const auto& label : func->labels) {
                if (label == "@entry") {
                    entryName = func->name;
                    break;
                }
            }
        }
    }
    return entryName;
}

void CodeGenContext::LogError(const std::string &message)
{
    std::cerr << "Error: " << message << std::endl;
    exit(1);
}

void CodeGenContext::LogWarning(const std::string &message)
{
#if DEBUG
    std::cerr << "Warning: " << message << std::endl;
#endif
}

llvm::Constant* CodeGenContext::createString(const std::string& text, const std::string& name) {
    // TODO: Дедупликация строк существует("stack overflow" к примеру), надо чисто добавить кэш и заебашить Constant*
    // Первым делом проверяю, что строка не содержит NUL-байт, потому что она может сломать некоторын функции как strlen, strcmp и т.д
    if (text.find('\0') != std::string::npos)
        throw std::runtime_error("Strings cannot contain a NUL byte");

    if (auto it = stringCache.find(text); it != stringCache.end())
        return it->second;

    // Создаю LLVM-константу для строки, которая хранит длину и данные строки в формате C ABI
    auto bytes = llvm::ConstantDataArray::getString(TheContext, text, true);


    // Создаю анонимную структуру, которая содержит длину строки и массив байтов с её содержимым
    auto value = llvm::ConstantStruct::getAnon({Builder.getInt64(text.size()), bytes});
    // ┌──────────────┬───┬───┬───┬───┬────┐  
    // │ i64 len = 4  │ t │ e │ x │ t │ \0 │
    // └──────────────┴───┴───┴───┴───┴────┘
    
    // Любая строка это глобальная констанат, чтобы её можно было юзать безопастно
    // @str = private constant { i64, [15 x i8] } { i64 14, [15 x i8] c"stack overflow\00" }, align 8

    auto global = new llvm::GlobalVariable(*TheModule, value->getType(), true,
        llvm::GlobalValue::PrivateLinkage, value, name);

    // Выравнивание глобальной переменной на 8 байт, чтобы C ABI не быковал
    global->setAlignment(llvm::Align(8));

    // Устанавливаю unnamed_addr, чтобы LLVM мог оптимизировать одинаковые строки
    global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

    llvm::Constant* indices[] = {Builder.getInt32(0), Builder.getInt32(1), Builder.getInt32(0)};
    auto ptr = llvm::ConstantExpr::getInBoundsGetElementPtr(value->getType(), global, indices);

    stringCache.emplace(text, ptr);

    // Получаю указатель на массив байтов внутри структуры, чтобы его можно было использовать как C-строку
    return ptr;
}
