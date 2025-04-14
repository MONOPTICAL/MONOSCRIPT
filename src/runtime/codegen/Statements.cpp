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
        llvm::Type* returnType = context.Builder.GetInsertBlock()->getParent()->getReturnType();
        llvm::Type* returnValueType = result->getType();

        if (!result) {
            result = context.Builder.CreateRetVoid();
        } else {
            if (returnValueType != returnType) {
                codeGen.LogWarning("Тип возвращаемого значения не совпадает с типом функции");
                if (returnValueType->isPointerTy()) {
                    codeGen.LogWarning("переходим по указателю и возвращаем голое значение(не указатель)");
                        
                    result = context.Builder.CreateLoad(returnType, result, "load");
                }
            }
            result = context.Builder.CreateRet(result);
            return result;
        }
    }
}