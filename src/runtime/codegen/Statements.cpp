#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"

namespace Statements {
    llvm::Value* handleReturnStatement(CodeGenContext& context, ReturnNode& node) {
        ASTGen codeGen(context);
        codeGen.LogWarning("visit для ReturnNode: вычисляю выражение и генерирую return");
    
        if (node.expression) {
            node.expression->accept(codeGen);
        }

        llvm::Value* result = codeGen.getResult();
        llvm::Function* currentFunction = context.Builder.GetInsertBlock()->getParent();
        llvm::Type* returnType = currentFunction->getReturnType();
        llvm::Type* returnValueType = result->getType();
        
        if (!result) 
        {
            if (returnType->isVoidTy()) 
            {
                return context.Builder.CreateRetVoid();
            } 
            else 
            {
                codeGen.LogWarning("Не удалось сгенерировать значение для return");
                llvm::Value* null = context.getNullValue();
                return context.Builder.CreateRet(null);
            }
        } 
        else 
        {
            if (returnValueType != returnType && !(returnType->isArrayTy())) 
            {
                codeGen.LogWarning("Тип возвращаемого значения не совпадает с типом функции");
                if (returnValueType->isPointerTy()) {
                    codeGen.LogWarning("переходим по указателю и возвращаем голое значение(не указатель)");
                        
                    result = context.Builder.CreateLoad(returnType, result, "load");
                    return context.Builder.CreateRet(result);
                }
            }

            if (returnValueType->isPointerTy()) {
                codeGen.LogWarning("возвращаем массив");
                result = context.Builder.CreateLoad(returnType, result, "load");
            }

            return context.Builder.CreateRet(result);
        }
        return nullptr; // <-- TODO: check
    }
}