#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"

namespace Statements {
    llvm::Value* handleReturnStatement(CodeGenContext& context, ReturnNode& node) {
        ASTGen codeGen(context);
        codeGen.LogWarning("visit для ReturnNode: вычисляю выражение и генерирую return");
        
        llvm::Value* result = nullptr;
        
        if (node.expression) {
            if (auto block = std::dynamic_pointer_cast<BlockNode>(node.expression)) {
                if (auto keyValue = std::dynamic_pointer_cast<KeyValueNode>(block->statements[0])) {
                    // Обработка map (словаря)
                    codeGen.LogWarning("visit для KeyValueNode: ");
                    codeGen.visit(*keyValue);
                    result = codeGen.getResult();
                } else {
                    // Обработка массива
                    codeGen.LogWarning("visit для Array BlockNode: " + std::to_string(block->statements.size()));
                    
                    // Получаем тип массива для функции
                    llvm::Function* currentFunction = context.Builder.GetInsertBlock()->getParent();
                    llvm::Type* returnType = currentFunction->getReturnType();
                    
                    // Проверяем, возвращает ли функция массив
                    // Для новой структуры - это ptr на array_struct
                    bool isArrayReturn = false;
                    llvm::Type* elementType = nullptr;
                    
                    if (returnType->isPointerTy()) {
                        // Проверяем, что это указатель на нашу структуру массива
                        llvm::StructType* arrayStruct = llvm::StructType::getTypeByName(context.TheContext, "array_struct");
                        if (arrayStruct) {
                            // Получаем тип элементов массива из контекста
                            // Примечание: в функциях тип элементов массива надо где-то хранить
                            // Например, в аннотации функции или метаданных
                            
                            // Определим тип элемента по первому элементу массива
                            if (!block->statements.empty()) {
                                auto firstElem = block->statements[0];
                                if (firstElem) {
                                    firstElem->accept(codeGen);
                                    elementType = codeGen.getResult()->getType();
                                    isArrayReturn = true;
                                }
                            }
                        }
                    }
                    
                    if (isArrayReturn && elementType) {
                        // Подготавливаем массив
                        std::vector<llvm::Value*> elements;
                        
                        // Обрабатываем каждый элемент массива
                        for (auto& elementNode : block->statements) {
                            if (!elementNode) continue;
                            
                            // Обрабатываем вложенные массивы рекурсивно
                            if (auto nestedBlock = std::dynamic_pointer_cast<BlockNode>(elementNode)) {
                                // Создаем временный ReturnNode для обработки вложенного массива
                                ReturnNode tempReturn(nestedBlock);
                                // Рекурсивно обрабатываем вложенный массив
                                llvm::Value* nestedArrayPtr = handleReturnStatement(context, tempReturn);
                                elements.push_back(nestedArrayPtr);
                            }
                            // Обрабатываем вызовы функций
                            else if (auto callNode = std::dynamic_pointer_cast<CallNode>(elementNode)) {
                                callNode->accept(codeGen);
                                llvm::Value* value = codeGen.getResult();
                                elements.push_back(value);
                            }
                            // Обычные элементы
                            else {
                                elementNode->accept(codeGen);
                                llvm::Value* elementValue = codeGen.getResult();
                                elements.push_back(elementValue);
                            }
                        }
                        
                        // Создаем массив через нашу функцию
                        llvm::Value* sizeValue = llvm::ConstantInt::get(
                            llvm::Type::getInt32Ty(context.TheContext), 
                            elements.size()
                        );
                        
                        // Создаем массив нужного размера
                        result = context.createArray(elementType, sizeValue);
                        
                        // Заполняем массив элементами
                        for (size_t i = 0; i < elements.size(); i++) {
                            llvm::Value* index = llvm::ConstantInt::get(
                                llvm::Type::getInt32Ty(context.TheContext), i);
                            context.setArrayElement(result, index, elements[i]);
                        }
                    } else {
                        // Если возвращаемый тип не массив, но выражение - блок,
                        // обрабатываем как последовательность команд
                        for (auto& stmt : block->statements) {
                            if (stmt) {
                                stmt->accept(codeGen);
                                result = codeGen.getResult();
                            }
                        }
                    }
                }
            } else {
                // Обычное выражение (не блок)
                node.expression->accept(codeGen);
                result = codeGen.getResult();
            }
        }
        
        if (!result) {
            if (context.Builder.GetInsertBlock()->getParent()->getReturnType()->isVoidTy()) {
                return context.Builder.CreateRetVoid();
            } else {
                codeGen.LogWarning("Не удалось сгенерировать значение для return");
                llvm::Value* null = context.getNullValue();
                return context.Builder.CreateRet(null);
            }
        } else {
            // Для массивов не нужно делать loadValueIfPointer,
            // так как они уже представлены как ptr
            if (!result->getType()->isPointerTy() || 
                !llvm::StructType::getTypeByName(context.TheContext, "array_struct")) {
                result = TypeConversions::loadValueIfPointer(context, result, "return_load");
            }
            
            if(node.expression->implicitCastTo) 
                result = TypeConversions::convertValueToType(context, result, 
                         context.Builder.GetInsertBlock()->getParent()->getReturnType(), "return_cast");
            
            return context.Builder.CreateRet(result);
        }
    }
}