#include "headers/CodeGenContext.h"
#include "../errors/headers/ErrorEngine.h"
#include <iostream>

llvm::Function* CodeGenContext::getOrDeclareFunction(const std::string& name, llvm::FunctionType* type) {
    llvm::Function* func = TheModule->getFunction(name); // Проверяем, существует ли функция
    if (!func) {
        // Функция не найдена в модуле, объявляем ее как внешнюю
        func = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, TheModule.get());
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
    panic->setDoesNotReturn();
    Builder.CreateCall(panic, {wideIndex, length, Builder.getInt32(ErrorEngine::getInstance().displayLine(line))});
    Builder.CreateUnreachable();

    Builder.SetInsertPoint(okBlock);
    return Builder.CreateGEP(elementType, getArrayData(array), wideIndex, "element_ptr");
}

void CodeGenContext::emitPanic(const std::string& message, int line) {
    llvm::Function* panic = getOrDeclareFunction("ms_panic",
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(TheContext),
            {llvm::PointerType::get(TheContext, 0), llvm::Type::getInt32Ty(TheContext)},
            false
        )
    );
    panic->setDoesNotReturn();

    const int sourceLine = ErrorEngine::getInstance().displayLine(line);
    Builder.CreateCall(panic, {Builder.CreateGlobalString(message), Builder.getInt32(sourceLine)});
    Builder.CreateUnreachable();
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