#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"
#include "../headers/TOMLstd.h"
#include "../../loader/headers/loader.h"
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

#if DEBUG
    std::cerr << "Left operand type: ";
    left->getType()->print(llvm::errs());
    std::cerr << "\nRight operand type: ";
    right->getType()->print(llvm::errs());
    std::cerr << "\nOperation: " << node.op << "\n";
#endif

    // Приведение типов
    if (node.left->implicitCastTo) {
        left = TypeConversions::applyImplicitCast(context, left, node.left->implicitCastTo, "left");
    }
    
    if (node.right->implicitCastTo) {
        right = TypeConversions::applyImplicitCast(context, right, node.right->implicitCastTo, "right");
    }
    
    if (left->getType() != right->getType()) {
        if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
            unsigned leftWidth = left->getType()->getIntegerBitWidth();
            unsigned rightWidth = right->getType()->getIntegerBitWidth();
            
            if (leftWidth > rightWidth) {
                right = context.Builder.CreateIntCast(
                    right, left->getType(), true, "implicit_cast_right"
                );
            } else {
                left = context.Builder.CreateIntCast(
                    left, right->getType(), true, "implicit_cast_left"
                );
            }
            
#if DEBUG
            std::cerr << "Это нихуя не правильный подход и надо исправлять TypeSymbolVisitor, это временное решение" << std::endl;
#endif
        }
    }

    if (node.op == "add" || node.op == "fadd") {
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
    else if (node.op == "sub" || node.op == "fsub") {
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
    else if (node.op == "mul" || node.op == "fmul") {
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
    else if (node.op == "sdiv" || node.op == "fdiv") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            // Обработка float деления...
            if (left->getType()->isIntegerTy()) {
                left = context.Builder.CreateSIToFP(left, llvm::Type::getFloatTy(context.TheContext), "int_to_float_left");
            }
            if (right->getType()->isIntegerTy()) {
                right = context.Builder.CreateSIToFP(right, llvm::Type::getFloatTy(context.TheContext), "int_to_float_right");
            }
            return context.Builder.CreateFDiv(left, right, "divtmp_float");
        } else {
            // Упрощенный вариант для целочисленного деления:
            llvm::Function* function = context.Builder.GetInsertBlock()->getParent();

            llvm::BasicBlock* currentBlock = context.Builder.GetInsertBlock();

            llvm::BasicBlock* trapBlock = llvm::BasicBlock::Create(context.TheContext, "trap", function);
            llvm::BasicBlock* divBlock = llvm::BasicBlock::Create(context.TheContext, "div", function);

            // Проверка на делитель == 0
            llvm::Value* isZero = context.Builder.CreateICmpEQ(
                right, llvm::ConstantInt::get(right->getType(), 0), "is_zero_check"
            );
            context.Builder.CreateCondBr(isZero, trapBlock, divBlock);

            // Код в trap-блоке
            context.Builder.SetInsertPoint(trapBlock);
            llvm::Function* trapFunc = llvm::Intrinsic::getOrInsertDeclaration(
                context.TheModule.get(), llvm::Intrinsic::trap
            );
            context.Builder.CreateCall(trapFunc);
            context.Builder.CreateUnreachable();

            // Код в блоке деления
            context.Builder.SetInsertPoint(divBlock);
            auto div = context.Builder.CreateSDiv(left, right, "divtmp_int");
            // Создаем переход к продолжению
            llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(
                context.TheContext, "div_continue", function
            );
            context.Builder.CreateBr(continueBlock);

            // Устанавливаем точку вставки на блок продолжения
            context.Builder.SetInsertPoint(continueBlock);

            // Создаем PHI-узел для результата деления
            llvm::PHINode* phi = context.Builder.CreatePHI(
                div->getType(), 1, "div_result"
            );
            phi->addIncoming(div, divBlock);

            return phi;
        }
    }
    else if (node.op == "srem" || node.op == "frem") {
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
        std::string TOML_path = loader::findTomlPath();
        if (TOML_path.empty()) {
            std::cerr << "Warning: Не удалось найти путь к TOML-файлу" << std::endl;
            return nullptr;
        }

        // Конкатенация строк через вызов функции из стандартной библиотеки
        llvm::Function* concatFunc = declareFunctionFromTOML("scat", context.TheModule.get(), 
                                                         context.TheContext, TOML_path);
        if (!concatFunc) {
            std::cerr << "Warning: Функция scat не найдена в стандартной библиотеке" << std::endl;
            return nullptr;
        }
        std::vector<llvm::Value*> args = {left, right};
        return context.Builder.CreateCall(concatFunc, args, "concat_result");
    }
    else if (node.op.starts_with("icmp")) {
        // Сравнение
        llvm::CmpInst::Predicate predicate;
        if (node.op == "icmp_eq") 
        { // ==
            predicate = llvm::CmpInst::ICMP_EQ;
        } 
        else if (node.op == "icmp_ne") 
        { // !=
            predicate = llvm::CmpInst::ICMP_NE;
        } 
        else if (node.op == "icmp_slt") 
        { // <
            predicate = llvm::CmpInst::ICMP_SLT;
        } 
        else if (node.op == "icmp_sgt") 
        { // >
            predicate = llvm::CmpInst::ICMP_SGT;
        } 
        else if (node.op == "icmp_sge") 
        { // >=
            predicate = llvm::CmpInst::ICMP_SGE;
        } 
        else if (node.op == "icmp_sle") 
        { // <=
            predicate = llvm::CmpInst::ICMP_SLE;
        } 
        else 
        {
#if DEBUG
            std::cerr << "Warning: Неизвестная операция сравнения: " << node.op << std::endl;
#endif
            return nullptr;
        }
        
        return context.Builder.CreateICmp(predicate, left, right, "cmptmp");
    }
    /*
    TODO: Сделать поддержку float
    else if (node.op.starts_with("fcmp")) {
        // Сравнение с плавающей точкой
        llvm::CmpInst::Predicate predicate;
        if (node.op == "fcmp_oeq") 
        { // ==
            predicate = llvm::CmpInst::FCMP_OEQ;
        } 
        else if (node.op == "fcmp_one") 
        { // !=
            predicate = llvm::CmpInst::FCMP_ONE;
        } 
        else if (node.op == "fcmp_olt") 
        { // <
            predicate = llvm::CmpInst::FCMP_OLT;
        } 
        else if (node.op == "fcmp_ogt") 
        { // >
            predicate = llvm::CmpInst::FCMP_OGT;
        } 
        else if (node.op == "fcmp_oge") 
        { // >=
            predicate = llvm::CmpInst::FCMP_OGE;
        } 
        else if (node.op == "fcmp_ole") 
        { // <=
            predicate = llvm::CmpInst::FCMP_OLE;
        } 
        else 
        {
            std::cerr << "Warning: Неизвестная операция сравнения с плавающей точкой: " << node.op << std::endl;
            return nullptr;
        }
        
        return context.Builder.CreateFCmp(predicate, left, right, "cmptmp");
    }
    */
#if DEBUG    
    std::cerr << "Warning: Неизвестная бинарная операция: " << node.op << std::endl;
#endif
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

#if DEBUG
    std::cerr << "Warning: Неизвестная унарная операция: " << node.op << std::endl;
#endif

    return nullptr;
}