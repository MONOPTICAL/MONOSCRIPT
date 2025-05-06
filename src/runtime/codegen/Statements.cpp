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
           result = TypeConversions::loadValueIfPointer(context, result, "return_load");
           if(node.expression->implicitCastTo) 
                result = TypeConversions::convertValueToType(context, result, returnValueType, "return_cast");

           return context.Builder.CreateRet(result);
        }
        return nullptr; // <-- TODO: check
    }
}