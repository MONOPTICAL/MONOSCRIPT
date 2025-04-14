#include "headers/CodeGenContext.h"

llvm::Function* CodeGenContext::getOrDeclareFunction(const std::string& name, llvm::FunctionType* type) {
    llvm::Function* func = TheModule->getFunction(name); // Проверяем, существует ли функция
    if (!func) {
        // Функция не найдена в модуле, объявляем ее как внешнюю
        func = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, TheModule.get());
    }
    return func;
}

llvm::Type* CodeGenContext::getLLVMType(std::shared_ptr<TypeNode> typeNode) {
    // TODO: Реализовать маппинг всех типов

    if (auto simpleType = std::dynamic_pointer_cast<SimpleTypeNode>(typeNode)) {
        if (simpleType->name == "i32") return llvm::Type::getInt32Ty(TheContext);
        if (simpleType->name == "i64") return llvm::Type::getInt64Ty(TheContext);
        if (simpleType->name == "float") return llvm::Type::getFloatTy(TheContext);
        if (simpleType->name == "bool") return llvm::Type::getInt1Ty(TheContext);
        if (simpleType->name == "string") return llvm::Type::getInt8Ty(TheContext)->getPointerTo(); // Строки как char*
        if (simpleType->name == "void") return llvm::Type::getVoidTy(TheContext);
        if (simpleType->name == "auto") return llvm::Type::getInt8Ty(TheContext)->getPointerTo(); // auto как указатель на i8
    }
    else if (auto genericType = std::dynamic_pointer_cast<GenericTypeNode>(typeNode)) {
        // Обработка параметризованных типов (generic)
    
        if (genericType->baseName == "array") {
            // array<i32>
            if (genericType->typeParameters.size() == 1) {
                llvm::Type* elementType = getLLVMType(genericType->typeParameters[0]);
                return llvm::ArrayType::get(elementType, 0); // 0 - размер массива, меняется в процессе синтеза IR
            }
        }
        else if (genericType->baseName == "map") {
            // map<string, i32>
            if (genericType->typeParameters.size() == 2) {
                llvm::Type* keyType = getLLVMType(genericType->typeParameters[0]);
                llvm::Type* valueType = getLLVMType(genericType->typeParameters[1]);
            }
        }
    }
    // TODO: Добавить обработку ошибок для неизвестных типов(например, если это класс который ранее был объявлен)
    return llvm::PointerType::getUnqual(TheContext);
}

std::shared_ptr<TypeNode> CodeGenContext::GetTypeByASTNode(std::shared_ptr<ASTNode> node) {
    if (auto typeNode = std::dynamic_pointer_cast<TypeNode>(node)) {
        return typeNode;
    }
    else if (auto numberNode = std::dynamic_pointer_cast<NumberNode>(node)) {
        return std::make_shared<SimpleTypeNode>("i32"); // TODO: добавить поддержку других типов чисел(i64)
    }
    else if (auto stringNode = std::dynamic_pointer_cast<StringNode>(node)) {
        return std::make_shared<SimpleTypeNode>("string");
    }
    else if (auto floatNode = std::dynamic_pointer_cast<FloatNumberNode>(node)) {
        return std::make_shared<SimpleTypeNode>("float");
    }
    else if (auto booleanNode = std::dynamic_pointer_cast<BooleanNode>(node)) {
        return std::make_shared<SimpleTypeNode>("bool");
    }
    else if (auto nullNode = std::dynamic_pointer_cast<NullNode>(node)) {
        return std::make_shared<SimpleTypeNode>("null");
    }
    throw std::runtime_error("Unknown ASTNode type for type inference : " + std::to_string(node->line) + ":" + std::to_string(node->column));
}