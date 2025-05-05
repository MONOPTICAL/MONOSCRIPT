#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"
#include "../headers/TOMLstd.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <string>
#include <vector>

llvm::Value* Expressions::handleBinaryOperation(CodeGenContext& context, BinaryOpNode& node, llvm::Value* left, llvm::Value* right) {
    left = TypeConversions::loadValueIfPointer(context, left, "left");
    right = TypeConversions::loadValueIfPointer(context, right, "right");

    if (node.left->implicitCastTo) {
        left = TypeConversions::applyImplicitCast(context, left, node.left->implicitCastTo, "left");
    }
    
    if (node.right->implicitCastTo) {
        right = TypeConversions::applyImplicitCast(context, right, node.right->implicitCastTo, "right");
    }
    
    if (node.op == "add") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            // Приводим оба операнда к типу float, если один из них float
            if (left->getType()->isIntegerTy()) {
                left = context.Builder.CreateSIToFP(left, llvm::Type::getFloatTy(context.TheContext), "int_to_float_left");
            }
            if (right->getType()->isIntegerTy()) {
                right = context.Builder.CreateSIToFP(right, llvm::Type::getFloatTy(context.TheContext), "int_to_float_right");
            }
            return context.Builder.CreateFAdd(left, right, "addtmp_float");
        } else {
            return context.Builder.CreateAdd(left, right, "addtmp_int");
        }
    } 
    else if (node.op == "sub") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            if (left->getType()->isIntegerTy()) {
                left = context.Builder.CreateSIToFP(left, llvm::Type::getFloatTy(context.TheContext), "int_to_float_left");
            }
            if (right->getType()->isIntegerTy()) {
                right = context.Builder.CreateSIToFP(right, llvm::Type::getFloatTy(context.TheContext), "int_to_float_right");
            }
            return context.Builder.CreateFSub(left, right, "subtmp_float");
        } else {
            return context.Builder.CreateSub(left, right, "subtmp_int");
        }
    } 
    else if (node.op == "mul") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            if (left->getType()->isIntegerTy()) {
                left = context.Builder.CreateSIToFP(left, llvm::Type::getFloatTy(context.TheContext), "int_to_float_left");
            }
            if (right->getType()->isIntegerTy()) {
                right = context.Builder.CreateSIToFP(right, llvm::Type::getFloatTy(context.TheContext), "int_to_float_right");
            }
            return context.Builder.CreateFMul(left, right, "multmp_float");
        } else {
            return context.Builder.CreateMul(left, right, "multmp_int");
        }
    } 
    else if (node.op == "sdiv") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            if (left->getType()->isIntegerTy()) {
                left = context.Builder.CreateSIToFP(left, llvm::Type::getFloatTy(context.TheContext), "int_to_float_left");
            }
            if (right->getType()->isIntegerTy()) {
                right = context.Builder.CreateSIToFP(right, llvm::Type::getFloatTy(context.TheContext), "int_to_float_right");
            }
            return context.Builder.CreateFDiv(left, right, "divtmp_float");
        } else {
            return context.Builder.CreateSDiv(left, right, "divtmp_int");
        }
    } 
    else if (node.op == "srem") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            if (left->getType()->isIntegerTy()) {
                left = context.Builder.CreateSIToFP(left, llvm::Type::getFloatTy(context.TheContext), "int_to_float_left");
            }
            if (right->getType()->isIntegerTy()) {
                right = context.Builder.CreateSIToFP(right, llvm::Type::getFloatTy(context.TheContext), "int_to_float_right");
            }
            return context.Builder.CreateFRem(left, right, "remtmp_float");
        } else {
            return context.Builder.CreateSRem(left, right, "remtmp_int");
        }
    } 
    else if (node.op == "scat") {
        // Конкатенация строк через вызов функции из стандартной библиотеки
        llvm::Function* concatFunc = declareFunctionFromTOML("scat", context.TheModule.get(), 
                                                         context.TheContext, STDLIB_TOML_PATH);
        if (!concatFunc) {
            std::cerr << "Warning: Функция scat не найдена в стандартной библиотеке" << std::endl;
            return nullptr;
        }
        std::vector<llvm::Value*> args = {left, right};
        return context.Builder.CreateCall(concatFunc, args, "concat_result");
    }
    
    std::cerr << "Warning: Неизвестная бинарная операция: " << node.op << std::endl;
    return nullptr;
}

llvm::Value* Expressions::handleUnaryOperation(CodeGenContext& context, UnaryOpNode& node, llvm::Value* operand) {
    operand = TypeConversions::loadValueIfPointer(context, operand, "operand");

    if (node.operand->implicitCastTo) {
        operand = TypeConversions::applyImplicitCast(context, operand, node.operand->implicitCastTo, "operand");
    }
    
    if (node.op == "neg") {  // Унарный минус
        if (operand->getType()->isFloatingPointTy()) {
            return context.Builder.CreateFNeg(operand, "negtmp");
        } else if (operand->getType()->isIntegerTy()) {
            return context.Builder.CreateNeg(operand, "negtmp");
        } else {
            std::cerr << "Warning: Неподдерживаемый тип для унарного минуса" << std::endl;
            return nullptr;
        }
    } 
    else if (node.op == "not") {  // Логическое отрицание
        if (operand->getType()->isIntegerTy(1)) {  // boolean
            return context.Builder.CreateNot(operand, "nottmp");
        } else if (operand->getType()->isIntegerTy()) {
            // Сначала преобразуем к bool (ненулевое -> true)
            llvm::Value* boolValue = context.Builder.CreateICmpNE(
                operand, 
                llvm::ConstantInt::get(operand->getType(), 0),
                "to_bool"
            );
            return context.Builder.CreateNot(boolValue, "nottmp");
        } else {
            std::cerr << "Warning: Неподдерживаемый тип для логического отрицания" << std::endl;
            return nullptr;
        }
    }
    
    std::cerr << "Warning: Неизвестная унарная операция: " << node.op << std::endl;
    return nullptr;
}