#include "headers/CodeGenContext.h"
#include "../errors/headers/ErrorEngine.h"
#include <iostream>

llvm::Function* CodeGenContext::getOrDeclareFunction(const std::string& name, llvm::FunctionType* type) {
    llvm::Function* func = TheModule->getFunction(name); // Проверяем, существует ли функция
    if (!func) {
        // Функция не найдена в модуле, объявляем ее как внешнюю
        const bool checked = name == "ms_panic_index" || name == "ms_panic_result" ||
            name == "ms_str_at" || name == "ms_map_get" || name == "ms_map_del" || name == "ms_map_next";
        if (checked) {
            std::vector<llvm::Type*> args(type->params().begin(), type->params().end());
            args.push_back(Builder.getPtrTy());
            type = llvm::FunctionType::get(type->getReturnType(), args, false);
        }
        func = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, TheModule.get());
        if (checked) func->addFnAttr("ms.panic_context");
    }
    return func;
}

llvm::Type* CodeGenContext::getLLVMType(std::shared_ptr<TypeNode> typeNode, llvm::LLVMContext& ctx) {
    // TODO: Реализовать хуйню для сложных типов как array<array<array<i16>>>

    if (auto simpleType = std::dynamic_pointer_cast<SimpleTypeNode>(typeNode)) {
        if (simpleType->name == "i1") return llvm::Type::getInt1Ty(ctx);
        if (simpleType->name == "i8") return llvm::Type::getInt8Ty(ctx);
        if (simpleType->name == "i16") return llvm::Type::getInt16Ty(ctx);
        if (simpleType->name == "i32") return llvm::Type::getInt32Ty(ctx);
        if (simpleType->name == "i64") return llvm::Type::getInt64Ty(ctx);
        if (simpleType->name == "float") return llvm::Type::getFloatTy(ctx);
        if (simpleType->name == "f64") return llvm::Type::getDoubleTy(ctx);
        if (simpleType->name == "char") return llvm::Type::getInt8Ty(ctx);
        if (simpleType->name == "string") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // Строки как char*
        if (simpleType->name == "void") return llvm::Type::getVoidTy(ctx);
        if (simpleType->name == "null") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // null как указатель на i8
        if (simpleType->name == "auto") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // auto как указатель на i8
    }
    else if (auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(typeNode)) {
        // Коллекции это объекты в куче, переменная хранит на них ссылку
        return llvm::PointerType::getUnqual(ctx);
    }
    
    return llvm::PointerType::getUnqual(ctx);
}

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
    llvm::StructType* arrayStruct = getArrayStruct();
    const llvm::DataLayout& layout = TheModule->getDataLayout();

    // В заголовке лежит ссылка на данные
    llvm::Value* header = allocateObject(Builder.getInt64(layout.getTypeAllocSize(arrayStruct)), true, "array");

    llvm::Value* bytes = Builder.CreateMul(
        Builder.CreateZExt(size, Builder.getInt64Ty()),
        Builder.getInt64(layout.getTypeAllocSize(elementType)),
        "array_bytes"
    );
    llvm::Value* data = allocateObject(bytes, elementType->isPointerTy(), "array_data");

    Builder.CreateStore(data, Builder.CreateStructGEP(arrayStruct, header, 0));
    Builder.CreateStore(size, Builder.CreateStructGEP(arrayStruct, header, 1));
    Builder.CreateStore(size, Builder.CreateStructGEP(arrayStruct, header, 2));
    return header;
}

llvm::Value* CodeGenContext::getArrayData(llvm::Value* array) {
    llvm::Value* field = Builder.CreateStructGEP(getArrayStruct(), array, 0);
    return Builder.CreateLoad(llvm::PointerType::get(TheContext, 0), field, "array_data");
}

llvm::Value* CodeGenContext::getArrayLength(llvm::Value* array) {
    llvm::Value* field = Builder.CreateStructGEP(getArrayStruct(), array, 1);
    return Builder.CreateLoad(llvm::Type::getInt32Ty(TheContext), field, "array_len");
}

llvm::Value* CodeGenContext::getArrayElementPtr(llvm::Value* array, llvm::Type* elementType, llvm::Value* index, int line) {
    llvm::Type* i64Type = llvm::Type::getInt64Ty(TheContext);
    llvm::Type* i32Type = llvm::Type::getInt32Ty(TheContext);

    // i1 это 0 или 1, а не 0 или -1. Отрицательный индекс после расширения станет огромным беззнаковым и не пройдёт проверку
    llvm::Value* wideIndex = index->getType()->isIntegerTy(1)
        ? Builder.CreateZExt(index, i64Type, "index")
        : Builder.CreateSExt(index, i64Type, "index");
    llvm::Value* length = getArrayLength(array);

    llvm::Function* function = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* failBlock = llvm::BasicBlock::Create(TheContext, "bounds.fail", function);
    llvm::BasicBlock* okBlock = llvm::BasicBlock::Create(TheContext, "bounds.ok", function);
    Builder.CreateCondBr(Builder.CreateICmpUGE(wideIndex, Builder.CreateZExt(length, i64Type)), failBlock, okBlock);

    Builder.SetInsertPoint(failBlock);
    llvm::Function* panic = getOrDeclareFunction("ms_panic_index",
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), {i64Type, i32Type, i32Type}, false));
    Builder.CreateCall(panic, {wideIndex, length, Builder.getInt32(ErrorEngine::getInstance().displayLine(line)), PanicContext});
    propagatePanic();

    Builder.SetInsertPoint(okBlock);
    return Builder.CreateGEP(elementType, getArrayData(array), wideIndex, "element_ptr");
}

void CodeGenContext::emitPanic(const std::string& message, int line) {
    auto raise = getOrDeclareFunction("ms_raise", llvm::FunctionType::get(Builder.getVoidTy(),
        {Builder.getPtrTy(), Builder.getInt32Ty(), Builder.getPtrTy()}, false));
    Builder.CreateCall(raise, {createString(message),
        Builder.getInt32(ErrorEngine::getInstance().displayLine(line)), PanicContext});
    propagatePanic();
}

llvm::StructType* CodeGenContext::getPanicStruct() {
    return llvm::StructType::get(TheContext, {Builder.getInt8Ty(), Builder.getInt32Ty(), Builder.getPtrTy(), Builder.getPtrTy()});
}

void CodeGenContext::propagatePanic() {
    if (!PanicHandlers.empty()) {
        Builder.CreateBr(PanicHandlers.back());
        return;
    }
    auto function = Builder.GetInsertBlock()->getParent();
    if (function->hasFnAttribute("ms.panic_context")) {
        if (function->getReturnType()->isVoidTy()) Builder.CreateRetVoid();
        else Builder.CreateRet(llvm::Constant::getNullValue(function->getReturnType()));
        return;
    }
    auto layout = getPanicStruct();
    auto message = Builder.CreateLoad(Builder.getPtrTy(), Builder.CreateStructGEP(layout, PanicContext, 2));
    auto line = Builder.CreateLoad(Builder.getInt32Ty(), Builder.CreateStructGEP(layout, PanicContext, 1));
    auto fatal = getOrDeclareFunction("ms_panic", llvm::FunctionType::get(Builder.getVoidTy(),
        {Builder.getPtrTy(), Builder.getInt32Ty()}, false));
    fatal->setDoesNotReturn();
    Builder.CreateCall(fatal, {message, line});
    Builder.CreateUnreachable();
}

void CodeGenContext::checkPanic() {
    auto function = Builder.GetInsertBlock()->getParent();
    auto fail = llvm::BasicBlock::Create(TheContext, "panic.propagate", function);
    auto ok = llvm::BasicBlock::Create(TheContext, "panic.clear", function);
    auto flag = Builder.CreateLoad(Builder.getInt8Ty(), Builder.CreateStructGEP(getPanicStruct(), PanicContext, 0));
    Builder.CreateCondBr(Builder.CreateICmpNE(flag, Builder.getInt8(0)), fail, ok);
    Builder.SetInsertPoint(fail);
    auto lineSlot = Builder.CreateStructGEP(getPanicStruct(), PanicContext, 1);
    auto line = Builder.CreateLoad(Builder.getInt32Ty(), lineSlot);
    Builder.CreateStore(Builder.CreateSelect(Builder.CreateICmpEQ(line, Builder.getInt32(0)),
        Builder.getInt32(ErrorEngine::getInstance().displayLine(CurrentLine)), line), lineSlot);
    propagatePanic();
    Builder.SetInsertPoint(ok);
}

llvm::Value* CodeGenContext::emitCall(llvm::Function* function, std::vector<llvm::Value*> arguments, const std::string& name) {
    const bool checked = function->hasFnAttribute("ms.panic_context");
    if (checked) arguments.push_back(PanicContext);
    auto value = Builder.CreateCall(function, arguments, function->getReturnType()->isVoidTy() ? "" : name);
    if (checked) checkPanic();
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
// Strings retain the C ABI data pointer; the preceding u64 stores the byte length.
llvm::Constant* CodeGenContext::createString(const std::string& text, const std::string& name) {
    if (text.find('\0') != std::string::npos)
        throw std::runtime_error("Strings cannot contain a NUL byte");
    auto bytes = llvm::ConstantDataArray::getString(TheContext, text, true);
    auto value = llvm::ConstantStruct::getAnon({Builder.getInt64(text.size()), bytes});
    auto global = new llvm::GlobalVariable(*TheModule, value->getType(), true,
        llvm::GlobalValue::PrivateLinkage, value, name);
    global->setAlignment(llvm::Align(8));
    llvm::Constant* indices[] = {Builder.getInt32(0), Builder.getInt32(1), Builder.getInt32(0)};
    return llvm::ConstantExpr::getInBoundsGetElementPtr(value->getType(), global, indices);
}
