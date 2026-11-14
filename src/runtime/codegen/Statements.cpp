#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"

namespace Statements {
    llvm::Value* handleReturnStatement(CodeGenContext& context, ReturnNode& node) {
        ASTGen codeGen(context);
        codeGen.LogWarning("visit для ReturnNode: вычисляю выражение и генерирую return");
        
        llvm::Value* result = nullptr;
        
        if (node.expression) {
            if (Arrays::isLiteral(node.expression)) {
                result = Arrays::emitLiteral(context, node.expression, context.CurrentReturnType);
            } else {
                // Обычное выражение (не блок)
                node.expression->accept(codeGen);
                result = codeGen.getResult();
            }
        }
        
        // В void-функции return без значения: выражение там это NullNode
        if (context.Builder.GetInsertBlock()->getParent()->getReturnType()->isVoidTy()) {
            return context.Builder.CreateRetVoid();
        }

        if (!result) {
            {
                codeGen.LogWarning("Не удалось сгенерировать значение для return");
                llvm::Value* null = context.getNullValue();
                return context.Builder.CreateRet(null);
            }
        } else {
            // Переменную загружаем, а ссылка на объект в куче останется как есть
            result = TypeConversions::loadValueIfPointer(context, result, "return_load");
            
            result = TypeConversions::convertNodeValue(context, result,
                     context.Builder.GetInsertBlock()->getParent()->getReturnType(), node.expression, context.CurrentReturnType);
            
            return context.Builder.CreateRet(result);
        }
    }
}