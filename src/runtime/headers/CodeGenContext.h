#ifndef CODEGENCONTEXT_H // Добавляем include guards
#define CODEGENCONTEXT_H

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <map>
#include <string>
#include <vector>
#include <memory> 

#include "../../parser/headers/AST.h"


class CodeGenContext {
public:
    llvm::LLVMContext                   TheContext; // Контекст LLVM(он отвечает за управление памятью)
    llvm::IRBuilder<>                   Builder; // IRBuilder - это класс, который помогает создавать IR-код
    std::unique_ptr<llvm::Module>       TheModule; // Модуль - это контейнер для IR-кода
    std::map<std::string, llvm::Value*> NamedValues; // Простая таблица символов для переменных/параметров
    std::map<llvm::Value*, llvm::Type*> arrayElementTypes;

    std::vector<llvm::BasicBlock*> loopEndBlocks;    // Стек для блоков выхода из цикла (для break)
    std::vector<llvm::BasicBlock*> loopCondBlocks;   // Стек для блоков условия цикла (для continue)

    llvm::Function*                     getOrDeclareFunction(const std::string& name, llvm::FunctionType* type);

    static llvm::Type*                  getLLVMType(std::shared_ptr<TypeNode> typeNode, llvm::LLVMContext& ctx);

    std::shared_ptr<TypeNode>           getTypeByASTNode(std::shared_ptr<ASTNode> node);

    llvm::Constant*                     getNoneValue() 
    {
        return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(TheContext), 0));
    }

    llvm::Constant*                     getNullValue() 
    {
        return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(TheContext), 0));
    }

    static std::string                  getTypeString(std::shared_ptr<TypeNode> typeNode)
    {
        return typeNode->inferredType->toString();
    }

    void                                pushLoopContext(llvm::BasicBlock* condBlock, llvm::BasicBlock* endBlock) 
    {
        if (condBlock) loopCondBlocks.push_back(condBlock);
        if (endBlock) loopEndBlocks.push_back(endBlock);
    }

    void                                popLoopContext() 
    {
        if (!loopCondBlocks.empty()) loopCondBlocks.pop_back();
        if (!loopEndBlocks.empty()) loopEndBlocks.pop_back();
    }

    llvm::BasicBlock*                   getCurrentLoopEndBlock() const 
    {
        if (loopEndBlocks.empty()) return nullptr;
        return loopEndBlocks.back();
    }

    llvm::BasicBlock*                   getCurrentLoopCondBlock() const 
    {
        if (loopCondBlocks.empty()) return nullptr;
        return loopCondBlocks.back();
    }

    void LogError(const std::string& message);
    void LogWarning(const std::string& message);

    // В класс CodeGenContext добавьте:
    llvm::Value* createArray(llvm::Type* elementType, llvm::Value* size);
    llvm::Value* getArrayElement(llvm::Value* array, llvm::Value* index);
    void setArrayElement(llvm::Value* array, llvm::Value* index, llvm::Value* value);
    void freeArray(llvm::Value* array);


    CodeGenContext(const std::string& moduleName = "ms_module") : Builder(TheContext) {
        TheModule = std::make_unique<llvm::Module>(moduleName, TheContext); // Создаем новый модуль
    }
};

#endif // CODEGENCONTEXT_H