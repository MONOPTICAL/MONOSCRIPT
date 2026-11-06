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

        std::shared_ptr<TypeNode> typeNode;
        if (node.expression->inferredType)
            typeNode = node.expression->inferredType;
        else 
            typeNode = node.type;
        

        llvm::Type* valueType = value->getType();
        
        if (!value) {
            codeGen.LogWarning("Не удалось сгенерировать код для выражения " + node.name);
            return nullptr;
        }

        value = TypeConversions::loadValueIfPointer(context, value);
        if(node.expression->implicitCastTo)
        {
            // Приведение типов
            value = TypeConversions::convertValueToType(context, value, varType);
            valueType = value->getType();
        }

        // Создаем переменную в таблице символов
        llvm::AllocaInst* alloca = context.createEntryAlloca(varType, node.name);
        context.NamedValues[node.name] = alloca;

        // Записываем значение в переменную
        context.Builder.CreateStore(value, alloca);
        return alloca;
    }
    
    llvm::Value* handleGlobalArrayVariable(CodeGenContext &context, VariableAssignNode &node, llvm::Type *varType, std::shared_ptr<BlockNode> blockExpr)
    {
        ASTGen codeGen(context);
        
        if (!blockExpr->statements.empty() && std::dynamic_pointer_cast<KeyValueNode>(blockExpr->statements[0]))
            throw std::runtime_error("Codegen Error: map пока не реализован: " + node.name);
    
        codeGen.LogWarning("Инициализация массива: " + node.name);
    
        // 1. Определяем тип элементов
        llvm::Type* elementType = nullptr;
        if (auto genType = std::dynamic_pointer_cast<GenericTypeNode>(node.type)) {
            if (genType->baseName == "array" && !genType->typeParameters.empty()) {
                elementType = context.getLLVMType(genType->typeParameters[0], context.TheContext);
            }
        } 
        
        if (!elementType && !blockExpr->statements.empty()) {
            // Если тип не указан явно, выводим из первого элемента
            auto firstElem = blockExpr->statements[0];
            firstElem->accept(codeGen);
            llvm::Value* firstValue = codeGen.getResult();
            elementType = firstValue->getType();
        }
    
        if (!elementType) {
            codeGen.LogWarning("Не удалось определить тип элементов массива " + node.name);
            elementType = llvm::Type::getInt32Ty(context.TheContext); // Fallback
        }
    
        // 2. Создаём или получаем тип структуры массива
        std::string typeName = "array_struct";
        llvm::StructType* arrayStruct = llvm::StructType::getTypeByName(context.TheContext, typeName);
        
        if (!arrayStruct) {
            arrayStruct = llvm::StructType::create(
                context.TheContext,
                {
                    llvm::PointerType::get(context.TheContext, 0),     // data: ptr
                    llvm::Type::getInt32Ty(context.TheContext),        // length: i32
                    llvm::Type::getInt32Ty(context.TheContext)         // capacity: i32
                },
                typeName
            );
        }
    
        // 3. Выделяем память для элементов массива
        size_t arraySize = blockExpr->statements.size();
        llvm::Value* dataPtr = nullptr;
        
        // Выделяем память для данных (либо через malloc, либо создаём глобальный массив)
        if (arraySize > 0) {
            // Создаём глобальный массив данных для инициализации
            std::vector<llvm::Constant*> elementConstants;
            bool allConstant = true;
            
            int index = 0;
            for (auto& stmt : blockExpr->statements) {
                if (!stmt) continue;
                
                // Специальная обработка для строк и других типов...
                if (auto stringNode = std::dynamic_pointer_cast<StringNode>(stmt)) {
                    llvm::Constant* strConst = llvm::ConstantDataArray::getString(
                        context.TheContext, stringNode->value, true);
                    llvm::GlobalVariable* strGlobal = new llvm::GlobalVariable(
                        *context.TheModule,
                        strConst->getType(),
                        true,
                        llvm::GlobalValue::PrivateLinkage,
                        strConst,
                        "str_elem_" + std::to_string(index)
                    );
                    llvm::Constant* strPtr = llvm::ConstantExpr::getBitCast(
                        strGlobal, 
                        llvm::PointerType::get(context.TheContext, 0)
                    );
                    elementConstants.push_back(strPtr);
                } 
                else {
                    stmt->accept(codeGen);
                    llvm::Value* elemValue = codeGen.getResult();
                    
                    if (llvm::Constant* constVal = llvm::dyn_cast<llvm::Constant>(elemValue)) {
                        elementConstants.push_back(constVal);
                    } else {
                        // Если хотя бы один элемент не константа, не сможем создать ConstantArray
                        allConstant = false;
                        elementConstants.push_back(llvm::UndefValue::get(elementType));
                        codeGen.LogWarning("Элемент массива не константа");
                    }
                }
                index++;
            }
            
            if (allConstant && !elementConstants.empty()) {
                // Создаём глобальный массив данных
                llvm::ArrayType* dataArrayType = llvm::ArrayType::get(elementType, arraySize);
                llvm::Constant* dataArray = llvm::ConstantArray::get(dataArrayType, elementConstants);
                
                llvm::GlobalVariable* dataGlobal = new llvm::GlobalVariable(
                    *context.TheModule,
                    dataArrayType,
                    true, // isConstant
                    llvm::GlobalValue::PrivateLinkage,
                    dataArray,
                    node.name + "_data"
                );
                
                dataPtr = llvm::ConstantExpr::getBitCast(
                    dataGlobal,
                    llvm::PointerType::get(context.TheContext, 0)
                );
            }
        }
        
        // Если не все элементы константы или массив пуст, используем malloc
        if (!dataPtr) {
            // Fallback на malloc если не все элементы константы
            llvm::Function* mallocFunc = context.getOrDeclareFunction("malloc",
                llvm::FunctionType::get(
                    llvm::PointerType::get(context.TheContext, 0),
                    llvm::Type::getInt32Ty(context.TheContext),
                    false
                )
            );
            
            llvm::Value* elemSize = llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(context.TheContext),
                context.TheModule->getDataLayout().getTypeAllocSize(elementType)
            );
            
            llvm::Value* bytesToAllocate = llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(context.TheContext),
                arraySize * context.TheModule->getDataLayout().getTypeAllocSize(elementType)
            );
            
            // В глобальном контексте нельзя вызывать функции, поэтому используем внешнюю инициализацию
            // Создаем глобальную функцию-инициализатор
            dataPtr = llvm::ConstantPointerNull::get(llvm::PointerType::get(context.TheContext, 0));
        }
    
        // 4. Создаем глобальную переменную структуры массива
        llvm::Constant* structInitializer = llvm::ConstantStruct::get(
            arrayStruct,
            {
                llvm::cast<llvm::Constant>(dataPtr),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.TheContext), arraySize),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.TheContext), arraySize)
            }
        );
        
        llvm::GlobalVariable* arrayVar = new llvm::GlobalVariable(
            *context.TheModule,
            arrayStruct,
            node.isConst,
            llvm::GlobalValue::PrivateLinkage,
            structInitializer,
            node.name
        );
        
        // 5. Сохраняем переменную в таблицу символов и тип элементов в таблицу типов
        context.NamedValues[node.name] = arrayVar;
        context.arrayElementTypes[arrayVar] = elementType;
    
        // 6. Если нужно динамически заполнить массив (не все элементы константы),
        // создаём функцию-инициализатор, которая будет вызвана до main()
        if (!dataPtr || llvm::isa<llvm::ConstantPointerNull>(dataPtr)) {
            // Это сложно реализовать в контексте глобальных переменных
            // и требует создания глобальных конструкторов
            codeGen.LogWarning("Для массива " + node.name + " требуется динамическая инициализация");
        }
        
        return arrayVar;
    }

    // Сборщик не видит память JIT, поэтому глобальную переменную со ссылкой показываем ему сами
    static void registerGCRoot(CodeGenContext& context, llvm::GlobalVariable* globalVar)
    {
        llvm::Type* ptrType = llvm::PointerType::get(context.TheContext, 0);
        llvm::Function* addRoots = context.getOrDeclareFunction("GC_add_roots",
            llvm::FunctionType::get(llvm::Type::getVoidTy(context.TheContext), {ptrType, ptrType}, false)
        );

        const uint64_t size = context.TheModule->getDataLayout().getTypeAllocSize(globalVar->getValueType());
        llvm::Value* end = context.Builder.CreateConstGEP1_64(llvm::Type::getInt8Ty(context.TheContext), globalVar, size);
        context.Builder.CreateCall(addRoots, {globalVar, end});
    }

    // Вызывается внутри стартовой функции: всё, что не константа, вычисляется при запуске программы
    llvm::Value* handleGlobalVariable(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType)
    {
        ASTGen codeGen(context);

        if (std::shared_ptr<BlockNode> blockNode = std::dynamic_pointer_cast<BlockNode>(node.expression)) {
            
            return handleGlobalArrayVariable(context, node, varType, blockNode);
        }

        node.expression->accept(codeGen);
        llvm::Value* initValue = codeGen.getResult();

        if (!initValue) {
            codeGen.LogWarning("Не удалось сгенерировать значение для глобальной переменной");
            return nullptr;
        }

        initValue = TypeConversions::loadValueIfPointer(context, initValue);
        initValue = TypeConversions::convertValueToType(context, initValue, varType);

        llvm::Constant* constantValue = llvm::dyn_cast<llvm::Constant>(initValue);

        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            *context.TheModule,
            varType,
            node.isConst && constantValue, // Переменную, в которую пишем при запуске, нельзя класть в константную память
            llvm::GlobalValue::PrivateLinkage,
            constantValue ? constantValue : llvm::Constant::getNullValue(varType),
            node.name
        );

        // Константная ссылка указывает на данные программы, сборщику там делать нечего
        if (varType->isPointerTy() && !globalVar->isConstant())
            registerGCRoot(context, globalVar);

        if (!constantValue)
            context.Builder.CreateStore(initValue, globalVar);

        context.NamedValues[node.name] = globalVar;
        return globalVar;
    }
}

llvm::Value *Declarations::handleSimpleReassignment(CodeGenContext &context, VariableReassignNode &node, llvm::Type *varType)
{
    ASTGen codeGen(context);
    // Генерируем код для выражения
    node.expression->accept(codeGen);
    llvm::Value* value = codeGen.getResult();
    if (!value) {
        return nullptr;
    }

    value = TypeConversions::loadValueIfPointer(context, value);
    value = TypeConversions::convertValueToType(context, value, varType);

    // Записываем значение в переменную
    context.Builder.CreateStore(value, context.NamedValues[node.name]);
    return context.NamedValues[node.name];
}
