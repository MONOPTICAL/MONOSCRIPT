#include "headers/ASTVisitors.h"
#include "headers/CodeGenHandlers.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>           // <<< Убедитесь, что этот include есть
#include <llvm/IR/DerivedTypes.h>   // <<< И этот тоже
#include <llvm/Support/raw_ostream.h> // <<< Добавлено для преобразования типа в строку
#include <string>
#include <string_view>
#include <vector>                   // <<< Добавлено для std::vector
#include "headers/TOMLstd.h" // <<< Добавлено для TOML

// Конструктор
ASTGen::ASTGen(CodeGenContext& context) : context(context), result(nullptr) {}

// Получить результат последнего посещенного узла
llvm::Value* ASTGen::getResult() const { 
    return result; 
}

// Вспомогательная функция для вывода предупреждений
void ASTGen::LogWarning(const std::string& message) {
    std::cerr << "Warning: " << message << std::endl;
}

// Реализации методов visitor для всех узлов AST
void ASTGen::visit(SimpleTypeNode& node) {
    LogWarning("visit не реализован для SimpleTypeNode: " + node.name);
    result = nullptr;
}

void ASTGen::visit(GenericTypeNode& node) {
    LogWarning("visit не реализован для GenericTypeNode: " + node.baseName);
    result = nullptr;
}

void ASTGen::visit(ProgramNode& node) {
    LogWarning("visit для ProgramNode: выполняю обработку " + std::to_string(node.body.size()) + " узлов");
    result = nullptr;
    
    for (auto& n : node.body) {
        if (n) {
            n->accept(*this);
            // Результат последнего узла становится результатом программы
        }
    }
}

void ASTGen::visit(FunctionNode& node) {
    LogWarning("visit не полностью реализован для FunctionNode: " + node.name);

    // Сохраняем текущую точку вставки
    llvm::BasicBlock* currentBlock = nullptr;
    llvm::Function* currentFunction = nullptr;
    if (context.Builder.GetInsertBlock()) {
        currentBlock = context.Builder.GetInsertBlock();
        currentFunction = currentBlock->getParent();
    }

    // Создаем минимальную заглушку функции
    std::vector<llvm::Type*> paramTypes;
    for (auto& param : node.parameters) {
        paramTypes.push_back(context.getLLVMType(param.first, context.TheContext));
    }
    
    llvm::Type* returnLLVMType = llvm::Type::getVoidTy(context.TheContext);
    if (node.returnType) {
        returnLLVMType = context.getLLVMType(node.returnType, context.TheContext);
    }
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnLLVMType, paramTypes, false);
    llvm::Function* func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, node.name, context.TheModule.get());
    
    // Создаем блок входа в функцию
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(
        context.TheContext, "entry", func);
    context.Builder.SetInsertPoint(entryBlock);
    
    // Сохраняем старую таблицу символов и создаем новую
    auto oldNamedValues = context.NamedValues;
    context.NamedValues.clear();
    
    // Обрабатываем параметры функции
    unsigned idx = 0;
    for (auto &arg : func->args()) {
        arg.setName(node.parameters[idx++].second);
    }
    
    // Генерируем тело функции, если оно есть
    if (node.body) {
        node.body->accept(*this);
    }
    
    // Добавляем return, если его нет
    if (!entryBlock->getTerminator()) {
        if (returnLLVMType->isVoidTy()) {
            context.Builder.CreateRetVoid();
        } else {
            context.Builder.CreateRet(llvm::Constant::getNullValue(returnLLVMType));
        }
    }

    if (currentBlock) {
        context.Builder.SetInsertPoint(currentBlock);
    }
    
    // Восстанавливаем старую таблицу символов
    context.NamedValues = oldNamedValues;
    
    result = func;
}

void ASTGen::visit(StructNode& node) {
    LogWarning("visit не реализован для StructNode: " + node.name);
    result = nullptr;
}

void ASTGen::visit(BlockNode& node) {
    LogWarning("visit для BlockNode: выполняю обработку " + std::to_string(node.statements.size()) + " инструкций");
    result = nullptr;
    
    for (auto& stmt : node.statements) {
        if (stmt) {
            stmt->accept(*this);
            // Результат последнего оператора становится результатом блока
        }
    }
}

void ASTGen::visit(VariableAssignNode& node) {
    LogWarning("visit для VariableAssignNode: " + node.name);
    
    // Проверяем, существует ли переменная в таблице символов
    if (context.NamedValues.find(node.name) != context.NamedValues.end()) {
        LogWarning("!!!Переменная " + node.name + " уже существует!!!");
        result = nullptr;
        return;
    }
    
    // Получаем тип переменной
    llvm::Type* varType;

    if (node.inferredType) 
        varType = context.getLLVMType(node.inferredType, context.TheContext);
    else 
        varType = context.getLLVMType(node.type, context.TheContext);
    

    LogWarning("Тип переменной " + node.name + " : " + node.type->toString());
    if (!varType) {
        LogWarning("Не удалось получить тип для переменной " + node.name);
        result = nullptr;
        return;
    }

    llvm::BasicBlock* currentBlock = context.Builder.GetInsertBlock();
    bool isGlobalContext = (currentBlock == nullptr);
    
    if (isGlobalContext) {
        result = Declarations::handleGlobalVariable(context, node, varType);
        return;
    }
    
    // Проверяем является ли выражение структурой данных (array или map)
    std::shared_ptr<BlockNode> blockExpr = std::dynamic_pointer_cast<BlockNode>(node.expression);
    if (blockExpr) {
        result = Arrays::handleArrayInitialization(context, node, varType, blockExpr);
        return;
    }
    
    // Обычное присваивание (не массив или массив с одним выражением)
    result = Declarations::handleSimpleAssignment(context, node, varType);
}

void ASTGen::visit(ReassignMemberNode& node) {
    LogWarning("visit не реализован для ReassignMemberNode");
    result = nullptr;
}

void ASTGen::visit(VariableReassignNode& node) {
    LogWarning("visit не реализован для VariableReassignNode: " + node.name);
    
    // Получаем тип переменной
    llvm::Type* varType = context.NamedValues[node.name]->getType();
    if (!varType) {
        LogWarning("Не удалось получить тип для переменной " + node.name);
        result = nullptr;
        return;
    }

    // Генерируем код для присваивания
    result = Declarations::handleSimpleReassignment(context, node, varType);
}

void ASTGen::visit(IfNode& node) {
    LogWarning("visit не реализован для IfNode");
    result = nullptr;
}

void ASTGen::visit(ForNode& node) {
    LogWarning("visit не реализован для ForNode: " + node.varName);
    result = nullptr;
}

void ASTGen::visit(WhileNode& node) {
    LogWarning("visit не реализован для WhileNode");
    result = nullptr;
}

void ASTGen::visit(ReturnNode& node) {
    result = Statements::handleReturnStatement(context, node);
}

void ASTGen::visit(CallNode& node) {
    llvm::Function* calleeFunc = declareFunctionFromTOML(node.callee, context.TheModule.get(), context.TheContext, STDLIB_TOML_PATH);
    if (!calleeFunc) {
        LogWarning("Неизвестная функция: " + node.callee);
        result = nullptr;
        return;
    }

    if (calleeFunc->arg_size() != node.arguments.size()) {
        LogWarning("Неверное количество аргументов для функции " + node.callee);
        result = nullptr;
        return;
    }

    std::vector<llvm::Value*> argsV;
    for (unsigned i = 0, e = node.arguments.size(); i != e; ++i) {
        if (!node.arguments[i]) {
            LogWarning("Пустой узел аргумента для функции " + node.callee);
            result = nullptr;
            return;
        }
        node.arguments[i]->accept(*this);
        llvm::Value* argVal = getResult();
        if (!argVal) {
            LogWarning("Не удалось сгенерировать аргумент " + std::to_string(i) + " для функции " + node.callee);
            result = nullptr;
            return;
        }

        // Приведение типа, если требуется
        if (node.arguments[i]->implicitCastTo) {
            llvm::Type* targetType = context.getLLVMType(node.arguments[i]->implicitCastTo, context.TheContext);
            llvm::Type* argType = argVal->getType();
            if (targetType && argType != targetType) {
                if (argType->isIntegerTy() && targetType->isFloatingPointTy()) {
                    argVal = context.Builder.CreateSIToFP(argVal, targetType, "cast_int2fp");
                } else if (argType->isFloatingPointTy() && targetType->isIntegerTy()) {
                    argVal = context.Builder.CreateFPToSI(argVal, targetType, "cast_fp2int");
                } else if (argType->isIntegerTy() && targetType->isIntegerTy()) {
                    LogWarning("!!!Неявное приведение целого числа к целому числу!!!");
                    argVal = context.Builder.CreateIntCast(argVal, targetType, true, "cast_int2int");
                } else if (argType->isPointerTy() && targetType->isPointerTy()) {
                    LogWarning("!!!Неявное приведение указателя к указателю!!!");
                    argVal = context.Builder.CreateBitCast(argVal, targetType, "cast_ptr2ptr");
                } 
            }
        }

        llvm::Type* expectedType = calleeFunc->getFunctionType()->getParamType(i);
        llvm::Type* argType = argVal->getType();
        if (argVal->getType()->isPointerTy()) {
            if (llvm::AllocaInst* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(argVal)) {
                llvm::Type* loadedType = allocaInst->getAllocatedType();
                argVal = context.Builder.CreateLoad(loadedType, argVal, "arg_load");
                argType = argVal->getType();
            } else if (llvm::GlobalVariable* globalVar = llvm::dyn_cast<llvm::GlobalVariable>(argVal)) {
                llvm::Type* valueType = globalVar->getValueType();
                if (valueType->isArrayTy() && valueType->getArrayElementType()->isIntegerTy(8)) {
                    // Cтрока ебанная
                    argVal = context.Builder.CreateInBoundsGEP(
                        valueType,
                        globalVar,
                        {llvm::ConstantInt::get(context.TheContext, llvm::APInt(32, 0)),
                         llvm::ConstantInt::get(context.TheContext, llvm::APInt(32, 0))}
                    );
                    argType = argVal->getType();
                } else {
                    // Обычная глобальная переменная
                    argVal = context.Builder.CreateLoad(valueType, argVal, "arg_load");
                    argType = argVal->getType();
                }
            }
        }

        if (argType != expectedType) {
            if (argType->isIntegerTy(1) && expectedType->isIntegerTy()) {
                argVal = context.Builder.CreateZExt(argVal, expectedType, "zext_i1_to_i32");
            } else if (argType->isIntegerTy() && expectedType->isIntegerTy(1)) {
                argVal = context.Builder.CreateTrunc(argVal, expectedType, "trunc_to_i1");
            } else if (argType->isIntegerTy() && expectedType->isIntegerTy()) {
                argVal = context.Builder.CreateIntCast(argVal, expectedType, true, "cast_int2int");
            } else if (argType->isFloatingPointTy() && expectedType->isFloatingPointTy()) {
                argVal = context.Builder.CreateFPCast(argVal, expectedType, "cast_fp2fp");
            } else {
                LogWarning("!!!Неизвестное неявное приведение аргумента " + std::to_string(i) + " для функции " + node.callee + "!!!");
            }
        }

        

        argsV.push_back(argVal);
    }

    llvm::Type* retType = calleeFunc->getReturnType();
    if (retType->isVoidTy()) {
        result = context.Builder.CreateCall(calleeFunc, argsV);
    } else {
        result = context.Builder.CreateCall(calleeFunc, argsV, "calltmp");
    }
}

void ASTGen::visit(BinaryOpNode& node) {
    LogWarning("visit не реализован для BinaryOpNode: " + node.op);
    result = nullptr;
}

void ASTGen::visit(UnaryOpNode& node) {
    LogWarning("visit не реализован для UnaryOpNode: " + node.op);
    result = nullptr;
}

void ASTGen::visit(IdentifierNode& node) {
    LogWarning("visit для IdentifierNode: " + node.name);
    // Базовая заглушка: ищем переменную в таблице символов
    if (context.NamedValues.find(node.name) != context.NamedValues.end()) {
        result = context.NamedValues[node.name];
        return;
    }
    result = nullptr;
}

void ASTGen::visit(NumberNode& node) {
    LogWarning("visit для NumberNode: " + std::to_string(node.value));

    llvm::Type* targetLLVMType = nullptr;

    if (node.implicitCastTo)
    {
        targetLLVMType = context.getLLVMType(node.implicitCastTo, context.TheContext); 
        if (!targetLLVMType) {
             LogWarning("Не удалось получить целевой тип LLVM из implicitCastTo. Используется i32 по умолчанию.");
             targetLLVMType = llvm::Type::getInt32Ty(context.TheContext);
        }
    }
    else
    {
        targetLLVMType = context.getLLVMType(node.inferredType, context.TheContext);
        LogWarning("Нет типа для неявного приведения. Используется i32 по умолчанию.");
    }

    if (targetLLVMType && targetLLVMType->isIntegerTy()) 
        result = llvm::ConstantInt::get(targetLLVMType, node.value, true);
    else if (targetLLVMType && targetLLVMType->isFloatingPointTy()) 
    {
        LogWarning("Неявное приведение NumberNode к типу с плавающей точкой. Создается ConstantFP.");
        double floatValue = static_cast<double>(node.value);
        result = llvm::ConstantFP::get(llvm::Type::getFloatTy(context.TheContext), llvm::APFloat(static_cast<float>(floatValue)));
    }
    else {
        // Обработка других типов или если targetLLVMType все еще nullptr
        LogWarning("Неподдерживаемый или неизвестный целевой тип для NumberNode. Используется i32 по умолчанию.");
        targetLLVMType = llvm::Type::getInt32Ty(context.TheContext);
        result = llvm::ConstantInt::get(targetLLVMType, node.value, true);
    }
}

void ASTGen::visit(FloatNumberNode& node) {
    LogWarning("visit для FloatNumberNode: " + std::to_string(node.value));
    result = llvm::ConstantFP::get(context.TheContext, llvm::APFloat(node.value));
}

void ASTGen::visit(StringNode& node) {
    LogWarning("visit для StringNode: \"" + node.value + "\"");
    result = context.Builder.CreateGlobalString(node.value, "str");
}

void ASTGen::visit(NullNode& node) {
    LogWarning("visit для NullNode");
    result = context.getNullValue();
}

void ASTGen::visit(NoneNode& node) {
    LogWarning("visit для NoneNode");
    result = context.getNoneValue();
}

void ASTGen::visit(KeyValueNode& node) {
    LogWarning("visit не реализован для KeyValueNode");
    result = nullptr;
}

void ASTGen::visit(BreakNode& node) {
    LogWarning("visit не реализован для BreakNode");
    result = nullptr;
}

void ASTGen::visit(ContinueNode& node) {
    LogWarning("visit не реализован для ContinueNode");
    result = nullptr;
}

void ASTGen::visit(AccessExpression& node) {
    LogWarning("visit не реализован для AccessExpression: " + node.memberName);
    result = nullptr;
}

void ASTGen::visit(ImportNode &node)
{
    LogWarning("visit не реализован для ImportNode");
    result = nullptr;
}

void ASTGen::visit(LambdaNode &node)
{
    LogWarning("visit не реализован для LambdaNode");
    result = nullptr;
}