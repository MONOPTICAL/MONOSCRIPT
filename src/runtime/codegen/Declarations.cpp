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
            } else if (node.type->toString() == "auto")  { // Другие преобразования типов
                varType = value->getType();
            }
        }
        
        // Создаем переменную в таблице символов
        llvm::AllocaInst* alloca = context.Builder.CreateAlloca(varType, nullptr, node.name);
        context.NamedValues[node.name] = alloca;
        
        // Записываем значение в переменную
        context.Builder.CreateStore(value, alloca);
        return alloca;
    }
    
    llvm::Value* handleGlobalStringVariable(CodeGenContext &context, VariableAssignNode &node, llvm::Type *varType, std::shared_ptr<StringNode> strNode)
    {
        // Создаем константный массив символов для строки
        llvm::Constant* strConstant = llvm::ConstantDataArray::getString(context.TheContext, strNode->value, true);
        
        // Создаем глобальную переменную с правильным типом (массив символов)
        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            *context.TheModule,
            strConstant->getType(),  // Тип - массив символов
            node.isConst,            // isConstant
            llvm::GlobalValue::CommonLinkage,
            strConstant,             // Инициализатор
            node.name
        );
        
        // Сохраняем переменную в таблице символов
        context.NamedValues[node.name] = globalVar;
        return globalVar;
    }

    llvm::Value* handleGlobalArrayVariable(CodeGenContext &context, VariableAssignNode &node, llvm::Type *varType, std::shared_ptr<BlockNode> blockExpr)
    {
        ASTGen codeGen(context);

        if (!std::dynamic_pointer_cast<KeyValueNode>(blockExpr->statements[0])) {
            codeGen.LogWarning("Инициализация массива");
        } else {
            // TODO: реализовать инициализацию map
            codeGen.LogWarning("Инициализация map не реализована");
            return nullptr;
        }
        
        if (!varType->isArrayTy()) {
            codeGen.LogWarning("Переменная " + node.name + " является массивом но динамический инициализированый");
            auto TypeNode = std::make_shared<GenericTypeNode>("array");
            TypeNode->typeParameters.push_back(context.getTypeByASTNode(blockExpr->statements[0]));
            varType = context.getLLVMType(TypeNode);
            if (!varType) {
                codeGen.LogWarning("Не удалось получить тип для массива " + node.name);
                return nullptr;
            }
            codeGen.LogWarning("Тип массива " + TypeNode->toString());
        }

        // Создаем массив констант для инициализации
        std::vector<llvm::Constant*> elements;
        for (int i = 0; i < blockExpr->statements.size(); ++i) {
            if (!blockExpr->statements[i]) {
                continue;
            }
            
            // Специальная обработка для строковых элементов
            if (auto stringNode = std::dynamic_pointer_cast<StringNode>(blockExpr->statements[i])) {
                // Создаём константную строку
                llvm::Constant* strConstant = llvm::ConstantDataArray::getString(
                    context.TheContext, stringNode->value, true);
                
                // Создаём глобальную строковую переменную
                llvm::GlobalVariable* strGlobal = new llvm::GlobalVariable(
                    *context.TheModule,
                    strConstant->getType(),
                    true, // isConstant
                    llvm::GlobalValue::PrivateLinkage,
                    strConstant,
                    "str" + std::to_string(i)
                );
                
                // Получаем указатель на неё (правильного типа)
                llvm::Constant* strPtr = llvm::ConstantExpr::getBitCast(
                    strGlobal, 
                    llvm::PointerType::get(llvm::Type::getInt8Ty(context.TheContext), 0)
                );
                
                elements.push_back(strPtr);
            }
            else if (auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(blockExpr->statements[i])) {
                // Если это идентификатор, получаем его значение из таблицы символов
                llvm::Value* idValue = context.NamedValues[identifierNode->name];
                if (!idValue) {
                    codeGen.LogWarning("Идентификатор " + identifierNode->name + " не найден в таблице символов");
                    elements.push_back(llvm::Constant::getNullValue(varType->getArrayElementType()));
                    continue;
                }
                
                if (llvm::GlobalVariable* globalVar = llvm::dyn_cast<llvm::GlobalVariable>(idValue)) {
                    llvm::Constant* castedPtr = llvm::ConstantExpr::getBitCast(
                        globalVar,
                        llvm::PointerType::get(llvm::Type::getInt8Ty(context.TheContext), 0)
                    );
                    elements.push_back(castedPtr);
                } else {
                    codeGen.LogWarning("Идентификатор " + identifierNode->name + " не найден в таблице символов");
                    elements.push_back(llvm::Constant::getNullValue(varType->getArrayElementType()));
                }
            }
            else {
                blockExpr->statements[i]->accept(codeGen);
                llvm::Value* elementValue = codeGen.getResult();
                
                // Преобразуем Value* в Constant*
                if (llvm::Constant* constVal = llvm::dyn_cast<llvm::Constant>(elementValue)) {
                    elements.push_back(constVal);
                } else {
                    codeGen.LogWarning("Элемент не является константой, используем 0");
                    elements.push_back(llvm::Constant::getNullValue(varType->getArrayElementType()));
                }
            }
        }
        
        // Создаем константный массив
        llvm::Type* elementType = varType->getArrayElementType();
        llvm::ArrayType* arrayType = llvm::ArrayType::get(elementType, elements.size());
        llvm::Constant* arrayConstant;
        
        if (!elements.empty()) {
            arrayConstant = llvm::ConstantArray::get(arrayType, elements);
        } else {
            arrayConstant = llvm::ConstantAggregateZero::get(varType);
        }
        
        // Создаем глобальную переменную с константным инициализатором
        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            *context.TheModule,
            arrayType,
            node.isConst,
            llvm::GlobalValue::CommonLinkage,
            arrayConstant,
            node.name
        );
        
        context.NamedValues[node.name] = globalVar;
        return globalVar;
    }

    llvm::Value* handleGlobalVariable(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType)
    {
        ASTGen codeGen(context);

        if (std::shared_ptr<StringNode> strNode = std::dynamic_pointer_cast<StringNode>(node.expression)) {

            return handleGlobalStringVariable(context, node, varType, strNode);
        }

        if (std::shared_ptr<BlockNode> blockNode = std::dynamic_pointer_cast<BlockNode>(node.expression)) {
            
            return handleGlobalArrayVariable(context, node, varType, blockNode);
        }

        node.expression->accept(codeGen);
        llvm::Value* initValue = codeGen.getResult();
        
        codeGen.LogWarning("Глобальный контекст: создаем глобальную переменную " + node.name);
    

        if (varType->isPointerTy())
        {
            // Если тип указатель, то получаем тип элемента и меняем тип переменной
            auto typeOfTheValue = context.getTypeByASTNode(node.expression);
            varType = context.getLLVMType(typeOfTheValue);
        }
        

        if (!initValue) {
            codeGen.LogWarning("Не удалось сгенерировать значение для глобальной переменной");
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