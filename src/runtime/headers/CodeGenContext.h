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


// Как структура лежит в памяти. Поля-объекты и строки это ссылки
struct StructLayout {
    StructNode*                                      node = nullptr;
    llvm::StructType*                                type = nullptr;
    llvm::Function*                                  constructor = nullptr;
    std::map<std::string, unsigned>                  fieldIndex;
    std::vector<std::shared_ptr<VariableAssignNode>> fields;
    bool                                             hasReferences = false; // Иначе память без ссылок: сборщику просматривать незачем
};

class CodeGenContext {
public:
    llvm::LLVMContext                   TheContext; // Контекст LLVM(он отвечает за управление памятью)
    llvm::IRBuilder<>                   Builder; // IRBuilder - это класс, который помогает создавать IR-код
    std::unique_ptr<llvm::Module>       TheModule; // Модуль - это контейнер для IR-кода
    std::map<std::string, llvm::Value*> NamedValues; // Простая таблица символов для переменных/параметров

    llvm::Function*                     StartFunction = nullptr; // Точка входа программы: инициализирует рантайм и глобальные переменные
    std::shared_ptr<TypeNode>           CurrentReturnType; // Что возвращает функция, которая генерируется сейчас
    std::map<std::string, StructLayout> Structs;

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
        return typeNode->toString();
    }

    // Память под переменную выделяется в начале функции, а не на каждой итерации цикла
    llvm::AllocaInst*                   createEntryAlloca(llvm::Type* type, const std::string& name)
    {
        llvm::Function* function = Builder.GetInsertBlock()->getParent();
        llvm::IRBuilder<> entryBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        return entryBuilder.CreateAlloca(type, nullptr, name);
    }

    void                                pushLoopContext(llvm::BasicBlock* condBlock,llvm::BasicBlock* endBlock) 
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

    // Останавливает программу с ошибкой выполнения. После этого текущий блок завершён
    void                                emitPanic(const std::string& message, int line);

    // Имя функции, с которой начинается программа: помеченная @entry или main
    static std::string                  findEntryName(const ProgramNode& program);

    void LogError(const std::string& message);
    void LogWarning(const std::string& message);

    // Массив это объект в куче: заголовок { ptr data, i32 length, i32 capacity } и отдельный блок данных.
    // Переменная хранит ссылку на заголовок, тип элементов известен только из типов в AST
    llvm::StructType*                   getArrayStruct();
    // Память без ссылок сборщик не просматривает и не обнуляет: вызывающий заполняет её сам
    llvm::Value*                        allocateObject(llvm::Value* bytes, bool hasReferences, const std::string& name);
    llvm::Value*                        createArray(llvm::Type* elementType, llvm::Value* size);
    llvm::Value*                        getArrayData(llvm::Value* array);
    llvm::Value*                        getArrayLength(llvm::Value* array);
    // Адрес элемента с проверкой границ, line для сообщения об ошибке
    llvm::Value*                        getArrayElementPtr(llvm::Value* array, llvm::Type* elementType, llvm::Value* index, int line);


    CodeGenContext(const std::string& moduleName = "ms_module") : Builder(TheContext) {
        TheModule = std::make_unique<llvm::Module>(moduleName, TheContext); // Создаем новый модуль
    }
};

#endif // CODEGENCONTEXT_H