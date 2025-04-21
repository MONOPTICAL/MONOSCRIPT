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

    llvm::Function*                     getOrDeclareFunction(const std::string& name, llvm::FunctionType* type);

    llvm::Type*                         getLLVMType(std::shared_ptr<TypeNode> typeNode);

    std::shared_ptr<TypeNode>           getTypeByASTNode(std::shared_ptr<ASTNode> node);

    llvm::Constant*                     getNoneValue() 
    {
        return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(TheContext), 0));
    }

    llvm::Constant*                     getNullValue() 
    {
        return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(TheContext), 0));
    }

    CodeGenContext(const std::string& moduleName = "ms_module") : Builder(TheContext) {
        TheModule = std::make_unique<llvm::Module>(moduleName, TheContext); // Создаем новый модуль
    }
};

#endif // CODEGENCONTEXT_H