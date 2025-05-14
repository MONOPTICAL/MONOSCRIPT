#include "headers/CodeGenContext.h"
#include <iostream>

llvm::Function* CodeGenContext::getOrDeclareFunction(const std::string& name, llvm::FunctionType* type) {
    llvm::Function* func = TheModule->getFunction(name); // Проверяем, существует ли функция
    if (!func) {
        // Функция не найдена в модуле, объявляем ее как внешнюю
        func = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, TheModule.get());
    }
    return func;
}

llvm::Type* CodeGenContext::getLLVMType(std::shared_ptr<TypeNode> typeNode, llvm::LLVMContext& ctx) {
    // TODO: Реализовать хуйню для сложных типов как array<array<array<i16>>>

    if (auto simpleType = std::dynamic_pointer_cast<SimpleTypeNode>(typeNode)) {
        if (simpleType->name == "i1") return llvm::Type::getInt1Ty(ctx);
        if (simpleType->name == "i8") return llvm::Type::getInt8Ty(ctx);
        if (simpleType->name == "i16") return llvm::Type::getInt16Ty(ctx);
        if (simpleType->name == "i32") return llvm::Type::getInt32Ty(ctx);
        if (simpleType->name == "i64") return llvm::Type::getInt64Ty(ctx);
        if (simpleType->name == "float") return llvm::Type::getFloatTy(ctx);
        if (simpleType->name == "string") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // Строки как char*
        if (simpleType->name == "void") return llvm::Type::getVoidTy(ctx);
        if (simpleType->name == "null") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // null как указатель на i8
        if (simpleType->name == "auto") return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0); // auto как указатель на i8
    }
    else if (auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(typeNode)) {
        // Обработка параметризованных типов
        if (genericType->baseName == "array") {
            // Создаём структуру для представления массива с метаданными
            llvm::Type* elementType = getLLVMType(genericType->typeParameters[0], ctx);
            
            // Структура Array { 
            //    T* data;       // Указатель на данные
            //    i32 length;    // Текущая длина
            //    i32 capacity;  // Выделенная емкость
            // }
            llvm::StructType* arrayStruct = llvm::StructType::create(ctx, {
                llvm::PointerType::get(elementType, 0),    // T* data
                llvm::Type::getInt32Ty(ctx),               // i32 length
                llvm::Type::getInt32Ty(ctx)                // i32 capacity
            }, "array_" + getTypeString(genericType->typeParameters[0]));
            
            // Возвращаем указатель на эту структуру
            return llvm::PointerType::get(arrayStruct, 0);
        }
        else if (genericType->baseName == "map") {
            llvm::Type* keyType = getLLVMType(genericType->typeParameters[0], ctx);
            llvm::Type* valueType = getLLVMType(genericType->typeParameters[1], ctx);
            
            // KV структура для пар ключ-значение
            llvm::StructType* kvPairType = llvm::StructType::create(ctx, {
                keyType,
                valueType
            }, "kvpair_" + getTypeString(genericType->typeParameters[0]) + 
                "_" + getTypeString(genericType->typeParameters[1]));
            
            // Структура Map { 
            //    KVPair* entries;  // Указатель на пары ключ-значение
            //    i32 count;        // Количество элементов
            //    i32 capacity;     // Выделенная емкость
            //    i32* hashTable;   // Хеш-таблица для быстрого поиска (опционально)
            // }
            llvm::StructType* mapStruct = llvm::StructType::create(ctx, {
                llvm::PointerType::get(kvPairType, 0),     // KVPair* entries
                llvm::Type::getInt32Ty(ctx),               // i32 count
                llvm::Type::getInt32Ty(ctx),               // i32 capacity
                llvm::PointerType::get(llvm::Type::getInt32Ty(ctx), 0) // i32* hashTable
            }, "map_" + getTypeString(genericType->typeParameters[0]) + 
                "_" + getTypeString(genericType->typeParameters[1]));
            
            return llvm::PointerType::get(mapStruct, 0);
        }
    }
    
    return llvm::PointerType::getUnqual(ctx);
}

std::shared_ptr<TypeNode> CodeGenContext::getTypeByASTNode(std::shared_ptr<ASTNode> node) {
    if (auto typeNode = std::dynamic_pointer_cast<TypeNode>(node)) {
        return typeNode;
    }
    else if (auto numberNode = std::dynamic_pointer_cast<NumberNode>(node)) {
        return std::make_shared<SimpleTypeNode>(numberNode->type->toString());
    }
    else if (auto stringNode = std::dynamic_pointer_cast<StringNode>(node)) {
        return std::make_shared<SimpleTypeNode>("string");
    }
    else if (auto floatNode = std::dynamic_pointer_cast<FloatNumberNode>(node)) {
        return std::make_shared<SimpleTypeNode>("float");
    }
    else if (auto nullNode = std::dynamic_pointer_cast<NullNode>(node)) {
        return std::make_shared<SimpleTypeNode>("null");
    }
    throw std::runtime_error("Unknown ASTNode type for type inference : " + std::to_string(node->line) + ":" + std::to_string(node->column));
}

// Создание нового массива заданного размера и типа
// Создание нового массива заданного размера и типа
llvm::Value* CodeGenContext::createArray(llvm::Type* elementType, llvm::Value* size) {
    // 1. Создаем структурный тип массива, если еще не создан
    std::string typeName = "array_struct";
    llvm::StructType* arrayStruct = llvm::StructType::getTypeByName(TheContext, typeName);
    
    if (!arrayStruct) {
        // Создаем новый тип структуры
        arrayStruct = llvm::StructType::create(
            TheContext,
            {
                llvm::PointerType::get(TheContext, 0),      // data: ptr (opaque pointer)
                llvm::Type::getInt32Ty(TheContext),         // length: i32
                llvm::Type::getInt32Ty(TheContext)          // capacity: i32
            },
            typeName
        );
    }
    
    // 2. Выделяем память для структуры массива
    llvm::Value* arrayPtr = Builder.CreateAlloca(arrayStruct, nullptr, "array_struct_ptr");
    
    // 3. Выделяем память для элементов массива
    llvm::Value* elemSize = Builder.getInt32(
        TheModule->getDataLayout().getTypeAllocSize(elementType)
    );
    llvm::Value* bytesToAllocate = Builder.CreateMul(size, elemSize, "bytes_to_alloc");
    
    // 4. Вызываем malloc для выделения памяти
    llvm::Function* mallocFunc = getOrDeclareFunction("malloc",
        llvm::FunctionType::get(
            llvm::PointerType::get(TheContext, 0),  // void* return
            llvm::Type::getInt32Ty(TheContext),     // size_t arg
            false
        )
    );
    
    llvm::Value* dataPtr = Builder.CreateCall(mallocFunc, bytesToAllocate, "data_ptr");
    
    // 5. Инициализируем поля структуры
    // data поле - сохраняем исходный указатель без битовой конвертации
    // (для opaque pointers не нужны приведения типа)
    llvm::Value* dataField = Builder.CreateStructGEP(arrayStruct, arrayPtr, 0, "data_field");
    Builder.CreateStore(dataPtr, dataField);
    
    // length поле
    llvm::Value* lengthField = Builder.CreateStructGEP(arrayStruct, arrayPtr, 1, "length_field");
    llvm::Value* sizeAsI32 = Builder.CreateIntCast(size, llvm::Type::getInt32Ty(TheContext), false);
    Builder.CreateStore(sizeAsI32, lengthField);
    
    // capacity поле
    llvm::Value* capacityField = Builder.CreateStructGEP(arrayStruct, arrayPtr, 2, "capacity_field");
    Builder.CreateStore(sizeAsI32, capacityField);
    
    // Сохраняем тип элементов для этого массива в таблицу типов
    arrayElementTypes[arrayPtr] = elementType;
    
    return arrayPtr;
}

// Получение элемента массива по индексу
llvm::Value* CodeGenContext::getArrayElement(llvm::Value* array, llvm::Value* index) {
    // 1. Получаем структуру массива
    llvm::StructType* arrayStruct = llvm::StructType::getTypeByName(TheContext, "array_struct");
    if (!arrayStruct) {
        LogError("Array structure type not defined");
        return llvm::UndefValue::get(Builder.getPtrTy());
    }
    
    // 2. Получаем указатель на данные из структуры
    llvm::Value* dataField = Builder.CreateStructGEP(arrayStruct, array, 0, "data_field");
    llvm::Value* dataPtr = Builder.CreateLoad(Builder.getPtrTy(), dataField, "data_ptr");
    
    // 3. Определяем тип элемента из таблицы типов
    llvm::Type* elementType = nullptr;
    if (arrayElementTypes.find(array) != arrayElementTypes.end()) {
        elementType = arrayElementTypes[array];
    } else {
        // Если тип неизвестен, используем generic i8 и делаем warning
        LogWarning("Element type for array unknown, using i8");
        elementType = llvm::Type::getInt8Ty(TheContext);
    }
    
    // 4. Получаем указатель на элемент с явным указанием типа
    llvm::Value* elementPtr = Builder.CreateGEP(elementType, dataPtr, index, "element_ptr");
    
    // 5. Загружаем значение
    return Builder.CreateLoad(elementType, elementPtr, "element_value");
}

// Установка элемента массива по индексу
void CodeGenContext::setArrayElement(llvm::Value* array, llvm::Value* index, llvm::Value* value) {
    // 1. Получаем структуру массива
    llvm::StructType* arrayStruct = llvm::StructType::getTypeByName(TheContext, "array_struct");
    if (!arrayStruct) {
        LogError("Array structure type not defined");
        return;
    }
    
    // 2. Получаем указатель на данные из структуры
    llvm::Value* dataField = Builder.CreateStructGEP(arrayStruct, array, 0, "data_field");
    llvm::Value* dataPtr = Builder.CreateLoad(Builder.getPtrTy(), dataField, "data_ptr");
    
    // 3. Определяем тип элемента
    llvm::Type* elementType = value->getType();
    
    // 4. Получаем указатель на элемент
    llvm::Value* elementPtr = Builder.CreateGEP(elementType, dataPtr, index, "element_ptr");
    
    // 5. Сохраняем значение
    Builder.CreateStore(value, elementPtr);
    
    // 6. Обновляем таблицу типов элементов, если нужно
    if (arrayElementTypes.find(array) == arrayElementTypes.end()) {
        arrayElementTypes[array] = elementType;
    }
}

// Освобождение памяти массива
void CodeGenContext::freeArray(llvm::Value* array) {
    // 1. Получаем структуру массива
    llvm::StructType* arrayStruct = llvm::StructType::getTypeByName(TheContext, "array_struct");
    if (!arrayStruct) {
        LogError("Array structure type not defined");
        return;
    }
    
    // 2. Получаем указатель на данные из структуры
    llvm::Value* dataField = Builder.CreateStructGEP(arrayStruct, array, 0, "data_field");
    llvm::Value* dataPtr = Builder.CreateLoad(Builder.getPtrTy(), dataField, "data_ptr");
    
    // 3. Вызываем free для освобождения данных
    llvm::Function* freeFunc = getOrDeclareFunction("free",
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(TheContext),
            llvm::PointerType::get(TheContext, 0),
            false
        )
    );
    
    // Для opaque pointers не требуется явное приведение типа
    Builder.CreateCall(freeFunc, dataPtr);
    
    // 4. Обнуляем указатель на данные (опционально)
    llvm::Value* nullPtr = llvm::ConstantPointerNull::get(Builder.getPtrTy());
    Builder.CreateStore(nullPtr, dataField);
    
    // 5. Обнуляем длину и емкость
    llvm::Value* lengthField = Builder.CreateStructGEP(arrayStruct, array, 1, "length_field");
    Builder.CreateStore(Builder.getInt32(0), lengthField);
    
    llvm::Value* capacityField = Builder.CreateStructGEP(arrayStruct, array, 2, "capacity_field");
    Builder.CreateStore(Builder.getInt32(0), capacityField);
    
    // 6. Удаляем из таблицы типов
    if (arrayElementTypes.find(array) != arrayElementTypes.end()) {
        arrayElementTypes.erase(array);
    }
}

void CodeGenContext::LogError(const std::string &message)
{
    std::cerr << "Error: " << message << std::endl;
    exit(1);
}

void CodeGenContext::LogWarning(const std::string &message)
{
    std::cerr << "Warning: " << message << std::endl;
}