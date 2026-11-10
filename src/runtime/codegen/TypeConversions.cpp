#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Intrinsics.h>
#include <cmath>
#include <limits>
#include <string>

// float в целое. Значение, которое не помещается, и NaN для LLVM это неопределённое поведение: получался мусор
static llvm::Value* emitFloatToInt(CodeGenContext& context, llvm::Value* value, llvm::Type* targetType, const std::string& name) {
    llvm::Type* floatType = value->getType();
    const unsigned width = targetType->getIntegerBitWidth();

    // Дробная часть отбрасывается, поэтому в i8 помещается и -128.9. Сравнения с NaN всегда ложны
    llvm::Value* truncated = context.Builder.CreateUnaryIntrinsic(llvm::Intrinsic::trunc, value);
    llvm::Value* fits;
    if (width == 1) {
        fits = context.Builder.CreateFCmpOEQ(value, value, "is_number");
    } else {
        const double limit = std::ldexp(1.0, width - 1);
        fits = context.Builder.CreateAnd(
            context.Builder.CreateFCmpOGE(truncated, llvm::ConstantFP::get(floatType, -limit)),
            context.Builder.CreateFCmpOLT(truncated, llvm::ConstantFP::get(floatType, limit)),
            "fits"
        );
    }

    llvm::Function* function = context.Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* errorBlock = llvm::BasicBlock::Create(context.TheContext, "cast.overflow", function);
    llvm::BasicBlock* okBlock = llvm::BasicBlock::Create(context.TheContext, "cast.ok", function);
    context.Builder.CreateCondBr(fits, okBlock, errorBlock);

    context.Builder.SetInsertPoint(errorBlock);
    context.emitPanic("float value out of range for i" + std::to_string(width), context.CurrentLine);

    context.Builder.SetInsertPoint(okBlock);
    // В i1 истина это всё, что не ноль: fptosi дал бы -1, который в i1 не помещается
    if (width == 1)
        return context.Builder.CreateFCmpUNE(truncated, llvm::ConstantFP::get(floatType, 0.0), name + "_fp2bool");
    return context.Builder.CreateFPToSI(value, targetType, name + "_fp2int");
}

llvm::Value* TypeConversions::loadValueIfPointer(CodeGenContext& context, llvm::Value* value, const std::string& name) {
    if (!value->getType()->isPointerTy()) {
        return value;  // Если не указатель, возвращаем как есть
    }
    
    std::string loadName = name.empty() ? "load" : name + "_load";
#if DEBUG
    std::cerr << "Warning: Применение loadValueIfPointer к " << value->getName().str() << std::endl;
    std::cerr << "Тип value: ";
    value->getType()->print(llvm::errs());
    std::cerr << std::endl;
#endif

    // Если значение alloca, то-есть локальная переменная на стеке
    if (llvm::AllocaInst* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(value)) { 
        llvm::Type* loadedType = allocaInst->getAllocatedType(); // Получаем тип, который выделен alloca
        if (loadedType->isArrayTy() && loadedType->getArrayElementType()->isIntegerTy(8)) { // Если это массив i8, обрабатываем как строку
            
            llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.TheContext), 0); // Тупо константа нуля
            llvm::Value* gep = context.Builder.CreateInBoundsGEP( // Создаём GEP для доступа к элементам массива i8 внутри alloca
                loadedType, // Тип, к которому применяем GEP
                allocaInst, // Указатель на alloca, к которому применяем GEP
                {zero, zero}, // Индексы для доступа к первому элементу массива i8 внутри alloca
                loadName + "_gep" // Имя для GEP
            );
            return gep; 
        }
        return context.Builder.CreateLoad(loadedType, value, loadName); // Если это не массив i8, просто загружаем значение из alloca
    } 
    // Если значение глобальная переменная
    else if (llvm::GlobalVariable* globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
        llvm::Type* valueType = globalVar->getValueType();
        // Специальная обработка для строк (массив i8)
        if (valueType->isArrayTy() && valueType->getArrayElementType()->isIntegerTy(8)) {
            llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.TheContext), 0);
            auto* gep = context.Builder.CreateInBoundsGEP(
                valueType, 
                globalVar, 
                {zero, zero}, 
                name + "_gep");
#if DEBUG
            llvm::errs() << "GEP создан: ";
            gep->print(llvm::errs());
            llvm::errs() << "\n";
#endif
            return gep;
        }
        return context.Builder.CreateLoad(valueType, value, loadName); // Если это не массив i8, просто загружаем значение из глобальной переменной
    }
 
    return value;  // Для других указателей оставляем как есть
}

llvm::Value* TypeConversions::convertValueToType(CodeGenContext& context, llvm::Value* value, llvm::Type* targetType, const std::string& name) {
    if (!value || !targetType || value->getType() == targetType) {
        return value;  // Если типы совпадают или что-то из входных параметров равно nullptr
    }

    std::string castName = name.empty() ? "cast" : name;

#if DEBUG
    std::cerr << "Warning: Применение convertValueToType к " << value->getName().str() << std::endl;
#endif

    // Целое в плавающую точку
    if (value->getType()->isIntegerTy() && targetType->isFloatingPointTy()) {
        // i1 это 0 или 1, со знаком истина превратилась бы в -1
        if (value->getType()->isIntegerTy(1))
            return context.Builder.CreateUIToFP(value, targetType, castName + "_i1tofp");
        return context.Builder.CreateSIToFP(value, targetType, castName + "_int2fp");
    } 
    // Плавающая точка в целое
    else if (value->getType()->isFloatingPointTy() && targetType->isIntegerTy()) {
        return emitFloatToInt(context, value, targetType, castName);
    } 
    // Целое в целое
    else if (value->getType()->isIntegerTy() && targetType->isIntegerTy()) {
        if (value->getType()->isIntegerTy(1)) {
            return context.Builder.CreateZExt(value, targetType, castName + "_i1toint");
        }
        // В i1 истина это всё, что не ноль: обрезка превратила бы 2 в ложь
        if (targetType->isIntegerTy(1))
            return context.Builder.CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0), castName + "_int2bool");
        return context.Builder.CreateIntCast(value, targetType, true, castName + "_int2int");
    }
    // Плавающая точка в плавающую точку
    else if (value->getType()->isFloatingPointTy() && targetType->isFloatingPointTy()) {
        // Литерал, который не помещается в float, это ошибка, а не бесконечность
        if (auto* literal = llvm::dyn_cast<llvm::ConstantFP>(value); literal && targetType->isFloatTy()
            && std::abs(literal->getValueAPF().convertToDouble()) > std::numeric_limits<float>::max() && !literal->isInfinity())
            throw std::runtime_error("Codegen Error: literal does not fit in float, use f64");
        return context.Builder.CreateFPCast(value, targetType, castName + "_fp2fp");
    }
    // Указатель в указатель
    else if (value->getType()->isPointerTy() && targetType->isPointerTy()) {
        return context.Builder.CreateBitCast(value, targetType, castName + "_ptr2ptr");
    }
    
    return value;
}

llvm::Value* TypeConversions::applyCharCast(CodeGenContext& context, ASTNode& node, llvm::Value* value) {
    if (!value || !node.inferredType || !node.implicitCastTo || node.inferredType->toString() != "char") return value;

    llvm::Type* target = context.getLLVMType(node.implicitCastTo, context.TheContext);
    if (!target->isIntegerTy() || target->isIntegerTy(8) || target->isIntegerTy(1)) return value;
    return context.Builder.CreateZExt(loadValueIfPointer(context, value), target, "char_to_int");
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