#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"
namespace Declarations 
{
    llvm::Value* handleSimpleAssignment(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType)
    {
        ASTGen codeGen(context);
        // Генерируем код для выражения
        node.expression->accept(codeGen);
        llvm::Value* value = codeGen.getResult();
        
        if (!value) {
            codeGen.LogWarning("Не удалось сгенерировать код для выражения " + node.name);
            return nullptr;
        }
        
        // Проверяем совместимость типов и выполняем преобразование при необходимости
        if (value->getType() != varType) {
            codeGen.LogWarning("Тип значения не совпадает с типом переменной " + node.name);
            if (varType->isFloatTy() && value->getType()->isIntegerTy()) { // int -> float
                value = context.Builder.CreateSIToFP(value, varType, "floatconv");
            } else if (varType->isIntegerTy() && value->getType()->isFloatTy()) { // float -> int
                value = context.Builder.CreateFPToSI(value, varType, "intconv");
            } else { // Другие преобразования типов
                codeGen.LogWarning("[CRITICAL]Типы не совпадают: " + std::to_string(varType->getTypeID()) + " != " + std::to_string(value->getType()->getTypeID()));
            }
        }
        
        // Создаем переменную в таблице символов
        llvm::AllocaInst* alloca = context.Builder.CreateAlloca(varType, nullptr, node.name);
        context.NamedValues[node.name] = alloca;
        
        // Записываем значение в переменную
        context.Builder.CreateStore(value, alloca);
        return alloca;
    }

    llvm::Value* handleGlobalVariable(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType, bool isConstant)
    {
        ASTGen codeGen(context);
        // Генерируем код для выражения
        node.expression->accept(codeGen);
        llvm::Value* result = codeGen.getResult();
        
        codeGen.LogWarning("Глобальный контекст: создаем глобальную переменную " + node.name);
    
        // Для глобальных переменных используем другой подход
        if (std::dynamic_pointer_cast<BlockNode>(node.expression)) {
            // Для массивов в глобальном контексте нужно использовать константные инициализаторы
            codeGen.LogWarning("Инициализация глобального массива не реализована");
            result = nullptr;
            return nullptr;
        }
        
        // Для простых значений создаем глобальную переменную
        node.expression->accept(codeGen);
        llvm::Value* initValue = codeGen.getResult();;
        
        if (!initValue) {
            codeGen.LogWarning("Не удалось сгенерировать значение для глобальной переменной");
            result = nullptr;
            return nullptr;
        }
        
        codeGen.LogWarning("Генерация значения для глобальной переменной " + node.name);
        
        // Для глобальной переменной используем GlobalVariable вместо alloca
        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            *context.TheModule,
            varType,
            node.isConst, // isConstant
            llvm::GlobalValue::CommonLinkage,
            llvm::Constant::getNullValue(varType), // Инициализатор
            node.name
        );
        
        // Если инициализатор - константа, используем её
        if (llvm::Constant* constVal = llvm::dyn_cast<llvm::Constant>(initValue)) {
            globalVar->setInitializer(constVal);
        }
        
        context.NamedValues[node.name] = globalVar;
        return globalVar;
    }
}