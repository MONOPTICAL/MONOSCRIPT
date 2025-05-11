#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"

namespace Arrays
{
    llvm::Value* handleArrayInitialization(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType, std::shared_ptr<BlockNode> blockExpr)
    {
        ASTGen codeGen(context);
        // Генерируем код для выражения
        node.expression->accept(codeGen);
        
        codeGen.LogWarning("Инициализация массива через блок для " + node.name);
    
        // Проверяем, является ли первый элемент KeyValueNode (для map)
        if (!blockExpr->statements.empty() && std::dynamic_pointer_cast<KeyValueNode>(blockExpr->statements[0])) {
            codeGen.LogWarning("Инициализация map не реализована");
            return nullptr;
        }
        
        // 1. Определяем тип элемента массива
        llvm::Type* elementType = nullptr;
        
        // Пытаемся получить тип элемента из объявления переменной
        if (auto genType = std::dynamic_pointer_cast<GenericTypeNode>(node.type)) {
            if (genType->baseName == "array" && !genType->typeParameters.empty()) {
                elementType = context.getLLVMType(genType->typeParameters[0], context.TheContext);
            }
        }
        
        // Если тип не указан явно, выводим из первого элемента
        if (!elementType && !blockExpr->statements.empty()) {
            auto firstElem = blockExpr->statements[0];
            firstElem->accept(codeGen);
            llvm::Value* firstValue = codeGen.getResult();
            elementType = firstValue->getType();
        }
        
        if (!elementType) {
            codeGen.LogWarning("Не удалось определить тип элементов массива " + node.name);
            elementType = llvm::Type::getInt32Ty(context.TheContext); // Fallback
        }
        
        // 2. Создаем структуру массива и выделяем память для данных
        llvm::Value* sizeValue = llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(context.TheContext), 
            blockExpr->statements.size()
        );
        
        llvm::Value* arrayPtr = context.createArray(elementType, sizeValue);
        
        // 3. Заполняем элементы массива
        for (size_t i = 0; i < blockExpr->statements.size(); i++) {
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
            
            // Индекс текущего элемента
            llvm::Value* index = llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(context.TheContext), i);
            
            // Используем нашу функцию для установки элемента
            context.setArrayElement(arrayPtr, index, elementValue);
        }
        
        // 4. Сохраняем переменную в таблицу символов и тип элементов в таблицу типов
        context.NamedValues[node.name] = arrayPtr;
        context.arrayElementTypes[arrayPtr] = elementType;
        
        return arrayPtr;
    }
}