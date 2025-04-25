#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"

namespace Arrays
{
    llvm::Value* handleArrayInitialization(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType, std::shared_ptr<BlockNode> blockExpr)
    {
        ASTGen codeGen(context);
        // Генерируем код для выражения
        node.expression->accept(codeGen);
        llvm::Value* result = codeGen.getResult();
        
        codeGen.LogWarning("Инициализация массива через блок для " + node.name);
    
        int arraySize = blockExpr->statements.size();
        
        if (!std::dynamic_pointer_cast<KeyValueNode>(blockExpr->statements[0])) {
            codeGen.LogWarning("Инициализация массива");
        } else {
            // TODO: реализовать инициализацию map
            codeGen.LogWarning("Инициализация map не реализована");
            result = nullptr;
            return nullptr;
        }
        
        if (!varType->isArrayTy()) {
            codeGen.LogWarning("Переменная " + node.name + " является массивом но динамический инициализированый");
            auto TypeNode = std::make_shared<GenericTypeNode>("array");
            TypeNode->typeParameters.push_back(context.getTypeByASTNode(blockExpr->statements[0]));
            varType = context.getLLVMType(TypeNode, context.TheContext);
            if (!varType) {
                codeGen.LogWarning("Не удалось получить тип для массива " + node.name);
                result = nullptr;
                return nullptr;
            }
            codeGen.LogWarning("Тип массива " + TypeNode->toString());
        }
        
        llvm::Type* elementType = varType->getArrayElementType();
        
        // Создаем массив нужного размера
        llvm::ArrayType* arrayType = llvm::ArrayType::get(elementType, arraySize);
        llvm::AllocaInst* arrayAlloca = context.Builder.CreateAlloca(arrayType, nullptr, node.name);
        context.NamedValues[node.name] = arrayAlloca;
        
        // Инициализируем каждый элемент массива
        for (int i = 0; i < arraySize; ++i) {
            if (!blockExpr->statements[i]) {
                codeGen.LogWarning("Пропускаю нулевой элемент массива #" + std::to_string(i));
                continue;
            }
            
            // Генерируем код для элемента массива
            blockExpr->statements[i]->accept(codeGen);
            llvm::Value* elementValue = codeGen.getResult();
            
            if (!elementValue) {
                codeGen.LogWarning("Не удалось сгенерировать код для элемента массива #" + std::to_string(i));
                continue;
            }
            
            // Создаем индексы для доступа к элементу массива
            std::vector<llvm::Value*> indices = {
                llvm::ConstantInt::get(context.TheContext, llvm::APInt(32, 0)),
                llvm::ConstantInt::get(context.TheContext, llvm::APInt(32, i))
            };
            
            // Получаем указатель на i-й элемент массива
            // %0 = getelementptr [1 x i32], ptr %four, i32 0, i32 0
            llvm::Value* elementPtr = context.Builder.CreateGEP(arrayType, arrayAlloca, indices);
            
            // Сохраняем значение в элемент массива
            // store i32 51, ptr %0, align 4
            context.Builder.CreateStore(elementValue, elementPtr);
        }
        
        return arrayAlloca;
    }
}