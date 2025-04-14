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
    llvm::LLVMContext TheContext; // Контекст LLVM(он отвечает за управление памятью)
    llvm::IRBuilder<> Builder; // IRBuilder - это класс, который помогает создавать IR-код
    std::unique_ptr<llvm::Module> TheModule; // Модуль - это контейнер для IR-кода
    std::map<std::string, llvm::Value*> NamedValues; // Простая таблица символов для переменных/параметров

    CodeGenContext(const std::string& moduleName = "ms_module") : Builder(TheContext) {
        TheModule = std::make_unique<llvm::Module>(moduleName, TheContext); // Создаем новый модуль
    }

    // Вспомогательная функция для получения или объявления внешней функции
    llvm::Function* getOrDeclareFunction(const std::string& name, llvm::FunctionType* type);

    // Вспомогательная функция для получения типа LLVM по TypeNode
    llvm::Type* getLLVMType(std::shared_ptr<TypeNode> typeNode);

    std::shared_ptr<TypeNode> GetTypeByASTNode(std::shared_ptr<ASTNode> node);

     // Функция для получения типа None/Null (зависит от вашей семантики)
    llvm::Constant* getNoneValue() {
        return llvm::ConstantPointerNull::get(llvm::Type::getInt8Ty(TheContext)->getPointerTo());
    }

    llvm::Constant* getNullValue() {
        return llvm::ConstantPointerNull::get(llvm::Type::getInt8Ty(TheContext)->getPointerTo());
    }
};

#endif // CODEGENCONTEXT_H