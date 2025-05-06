#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <string>

llvm::Value* TypeConversions::loadValueIfPointer(CodeGenContext& context, llvm::Value* value, const std::string& name) {
    if (!value->getType()->isPointerTy()) {
        return value;  // Если не указатель, возвращаем как есть
    }
    
    std::string loadName = name.empty() ? "load" : name + "_load";
#if DEBUG
    std::cerr << "!!@Warning: Применение loadValueIfPointer к " << value->getName().str() << std::endl;
    std::cerr << "Тип value: ";
    value->getType()->print(llvm::errs());
    std::cerr << std::endl;
#endif

    if (llvm::AllocaInst* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(value)) {
        llvm::Type* loadedType = allocaInst->getAllocatedType();
        if (loadedType->isArrayTy() && loadedType->getArrayElementType()->isIntegerTy(8)) {
            
            llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.TheContext), 0);
            llvm::Value* gep = context.Builder.CreateInBoundsGEP(
                loadedType,
                allocaInst,
                {zero, zero},
                loadName + "_gep"
            );
            return gep;
        }
        return context.Builder.CreateLoad(loadedType, value, loadName);
    } 
    else if (llvm::GlobalVariable* globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
        llvm::Type* valueType = globalVar->getValueType();
        // Специальная обработка для строк (массив i8)
        if (valueType->isArrayTy() && valueType->getArrayElementType()->isIntegerTy(8)) {
            llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.TheContext), 0);
            return context.Builder.CreateInBoundsGEP(
                valueType, 
                globalVar, 
                {zero, zero}, 
                name + "_gep");        
        }
        return context.Builder.CreateLoad(valueType, value, loadName);
    }

    return value;  // Для других указателей оставляем как есть
}

llvm::Value* TypeConversions::convertValueToType(CodeGenContext& context, llvm::Value* value, llvm::Type* targetType, const std::string& name) {
    if (!value || !targetType || value->getType() == targetType) {
        return value;  // Если типы совпадают или что-то из входных параметров равно nullptr
    }

    std::string castName = name.empty() ? "cast" : name;
    
    // Целое в плавающую точку
    if (value->getType()->isIntegerTy() && targetType->isFloatingPointTy()) {
        return context.Builder.CreateSIToFP(value, targetType, castName + "_int2fp");
    } 
    // Плавающая точка в целое
    else if (value->getType()->isFloatingPointTy() && targetType->isIntegerTy()) {
        return context.Builder.CreateFPToSI(value, targetType, castName + "_fp2int");
    } 
    // Целое в целое
    else if (value->getType()->isIntegerTy() && targetType->isIntegerTy()) {
        if (value->getType()->isIntegerTy(1)) {
            return context.Builder.CreateZExt(value, targetType, castName + "_i1toint");
        }
        return context.Builder.CreateIntCast(value, targetType, true, castName + "_int2int");
    }
    // Плавающая точка в плавающую точку
    else if (value->getType()->isFloatingPointTy() && targetType->isFloatingPointTy()) {
        return context.Builder.CreateFPCast(value, targetType, castName + "_fp2fp");
    }
    // Указатель в указатель
    else if (value->getType()->isPointerTy() && targetType->isPointerTy()) {
        return context.Builder.CreateBitCast(value, targetType, castName + "_ptr2ptr");
    }
    
    // Если не удалось преобразовать
    std::cerr << "Warning: Невозможно преобразовать тип в целевой" << std::endl;
    return value;
}

llvm::Value* TypeConversions::applyImplicitCast(CodeGenContext& context, llvm::Value* value, std::shared_ptr<TypeNode> targetTypeNode, const std::string& name) {
    if (!targetTypeNode || !value) {
        return value;
    }
    
    llvm::Type* targetType = context.getLLVMType(targetTypeNode, context.TheContext);
    if (!targetType) {
        return value;
    }
    
    return convertValueToType(context, value, targetType, name);
}