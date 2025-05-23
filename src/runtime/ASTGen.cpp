#include "headers/ASTVisitors.h"
#include "headers/CodeGenHandlers.h"
#include "../loader/headers/loader.h"
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
#if DEBUG
    std::cerr << "Warning: " << message << std::endl;
#endif
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
#if DEBUG
    context.TheModule->print(llvm::outs(), nullptr);
#endif
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
    
    bool isMain = (node.name == "main" || std::find(node.labels.begin(), node.labels.end(), "@entry") != node.labels.end());
    llvm::Type* returnLLVMType = llvm::Type::getVoidTy(context.TheContext);
    if (node.returnType) {
        returnLLVMType = context.getLLVMType(node.returnType, context.TheContext);
    }
    if (isMain) {
        returnLLVMType = llvm::Type::getInt32Ty(context.TheContext);
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
    
    // Обрабатываем параметры функции
    unsigned idx = 0;
    for (auto &arg : func->args()) {
        arg.setName(node.parameters[idx++].second);
        context.NamedValues[arg.getName().str()] = &arg;
    }

    // Генерируем тело функции, если оно есть
    if (node.body) {
        node.body->accept(*this);
    }

    // Добавляем return, если его нет
    llvm::BasicBlock* currentInsertionBlock = context.Builder.GetInsertBlock();
    LogWarning("Текущий блок не имеет терминирующей инструкции: " + currentInsertionBlock->getName().str());
    if (!entryBlock->getTerminator()) { // Если текущий блок - это entryBlock
#if DEBUG
        context.TheModule->print(llvm::outs(), nullptr);
#endif
        if (isMain) {
            // main всегда возвращает 0, если пользователь не указал return
            context.Builder.CreateRet(llvm::ConstantInt::get(returnLLVMType, 0));
        } else if (returnLLVMType->isVoidTy()) {
            context.Builder.CreateRetVoid();
        } 
        // TODO: Добавить тут обработки если надо
    }

    // Новая проверка для текущего блока
    if (currentInsertionBlock && currentInsertionBlock != entryBlock && !currentInsertionBlock->getTerminator()) {
        context.Builder.SetInsertPoint(currentInsertionBlock);
        if (isMain) {
            context.Builder.CreateRet(llvm::ConstantInt::get(returnLLVMType, 0));
        } else if (returnLLVMType->isVoidTy()) {
            context.Builder.CreateRetVoid();
        }
        // TODO: Добавить тут обработки если надо
    }

    
    // Восстанавливаем старую таблицу символов
    context.NamedValues = oldNamedValues;
    
    result = func;

    context.Builder.ClearInsertionPoint();
}

void ASTGen::visit(StructNode& node) {
    LogWarning("visit не реализован для StructNode: " + node.name);
    result = nullptr;
}

void ASTGen::visit(BlockNode& node) {
    LogWarning("visit для BlockNode: выполняю обработку " + std::to_string(node.statements.size()) + " инструкций");
    result = nullptr;

    // Сохраняем точку вставки до генерации блока
    llvm::BasicBlock* prevBlock = context.Builder.GetInsertBlock();

    bool isInEntryBlock = (prevBlock && prevBlock->getName() == "entry");

    for (auto& stmt : node.statements) {
        if (!stmt) continue;

        stmt->accept(*this);

        // Если это вложенная функция, то чтобы не терялась точка вставки
        if (dynamic_cast<FunctionNode*>(stmt.get()) && prevBlock) {
            context.Builder.SetInsertPoint(prevBlock);
        }
        if (dynamic_cast<IfNode*>(stmt.get()) && prevBlock) {
            prevBlock = context.Builder.GetInsertBlock();
            context.Builder.SetInsertPoint(prevBlock);
        }
        if (dynamic_cast<WhileNode*>(stmt.get()) && prevBlock) {
            prevBlock = context.Builder.GetInsertBlock();
            context.Builder.SetInsertPoint(prevBlock);
        }
    }

    // Восстанавливаем точку вставки после генерации блока
    if (prevBlock) 
    {
        context.Builder.SetInsertPoint(prevBlock);
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

#if DEBUG
    std::cerr << "Имя " << node.name << std::endl;
#endif

    if (node.inferredType) 
        varType = context.getLLVMType(node.inferredType, context.TheContext);
    else if(node.type)
        varType = context.getLLVMType(node.type, context.TheContext);
    else 
        LogWarning("Не удалось получить тип для переменной " + node.name);
    
#if DEBUG
    std::cerr << "Тип переменной " << node.name << " : " << node.type->toString() << std::endl;
#endif

    LogWarning("Тип переменной поставлен");
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
#if DEBUG
        std::cerr << "Обнаружено выражение блока для переменной " << node.name << std::endl;
#endif
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
    LogWarning("visit для IfNode");

    // Генерируем код для условия
    node.condition->accept(*this);
    llvm::Value* condValue = getResult();
    if (!condValue) {
        LogWarning("Ошибка: не удалось вычислить условие");
        result = nullptr;
        return;
    }

    if (!condValue->getType()->isIntegerTy(1)) {
        condValue = context.Builder.CreateICmpNE(
            condValue, 
            llvm::Constant::getNullValue(condValue->getType()),
            "ifcond"
        );
    }

    llvm::Function* function = context.Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(context.TheContext, "then", function);
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(context.TheContext, "ifcont", function);
    llvm::BasicBlock* elseBlock = nullptr;

    if (node.elseBlock) {
        elseBlock = llvm::BasicBlock::Create(context.TheContext, "else", function);
        context.Builder.CreateCondBr(condValue, thenBlock, elseBlock);
    } else {
        context.Builder.CreateCondBr(condValue, thenBlock, mergeBlock);
    }

    context.Builder.SetInsertPoint(thenBlock);
    node.thenBlock->accept(*this);
    if (!context.Builder.GetInsertBlock()->getTerminator())
        context.Builder.CreateBr(mergeBlock);

    if (node.elseBlock) {
        context.Builder.SetInsertPoint(elseBlock);
        node.elseBlock->accept(*this);
        if (!context.Builder.GetInsertBlock()->getTerminator())
            context.Builder.CreateBr(mergeBlock);
    }

    context.Builder.SetInsertPoint(mergeBlock);
    result = nullptr;
}

void ASTGen::visit(ForNode& node) {
    LogWarning("visit не реализован для ForNode: " + node.varName);
    result = nullptr;
}

void ASTGen::visit(WhileNode& node) {
    LogWarning("visit для WhileNode");

    llvm::Function* function = context.Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* loopCondBlock = llvm::BasicBlock::Create(context.TheContext, "loop.cond", function);
    llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(context.TheContext, "loop.body", function);
    llvm::BasicBlock* loopEndBlock = llvm::BasicBlock::Create(context.TheContext, "loop.end", function);

    // Сохраняем контекст цикла
    context.pushLoopContext(loopCondBlock, loopEndBlock);

    context.Builder.CreateBr(loopCondBlock);

    context.Builder.SetInsertPoint(loopCondBlock);
    node.condition->accept(*this);
    llvm::Value* condValue = getResult();
    if (!condValue) {
        LogWarning("Ошибка: не удалось вычислить условие для WhileNode");
        if (!context.Builder.GetInsertBlock()->getTerminator()) {
            context.Builder.CreateBr(loopEndBlock);
        }
        context.popLoopContext(); // Не забываем очистить стек при ошибке
        result = nullptr;
        return;
    }

    if (!condValue->getType()->isIntegerTy(1)) {
        condValue = context.Builder.CreateICmpNE(
            condValue,
            llvm::Constant::getNullValue(condValue->getType()),
            "whilecond.tobool"
        );
    }
    context.Builder.CreateCondBr(condValue, loopBodyBlock, loopEndBlock);

    context.Builder.SetInsertPoint(loopBodyBlock);
    if (node.body) {
        node.body->accept(*this);
    }
    
    if (!context.Builder.GetInsertBlock()->getTerminator()) {
        context.Builder.CreateBr(loopCondBlock);
    }

    context.Builder.SetInsertPoint(loopEndBlock);

    // Восстанавливаем/очищаем контекст цикла
    context.popLoopContext();

    result = nullptr;
}

void ASTGen::visit(ReturnNode& node) {
    result = Statements::handleReturnStatement(context, node);
}

void ASTGen::visit(CallNode& node) {
    LogWarning("visit не реализован для CallNode: " + node.callee);
    // Сначала в модуле ищем хуйню
    llvm::Function* calleeFunc = context.TheModule->getFunction(node.callee);

    // Если не нашли пробуем объявить через TOML
    if (!calleeFunc) {
        std::string TOML_path = loader::findTomlPath();
        if (TOML_path.empty()) {
            LogWarning("Не удалось найти TOML файл для функции " + node.callee);
            result = nullptr;
            return;
        }
        
        calleeFunc = declareFunctionFromTOML(node.callee, context.TheModule.get(), context.TheContext, TOML_path);
    }

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

void ASTGen::visit(ModuleMark &node)
{
    LogWarning("visit для ModuleMark: " + node.moduleName);
    this->currentModuleName = node.moduleName;
}

void ASTGen::visit(BinaryOpNode& node) {
    LogWarning("Обработка BinaryOpNode: " + node.op);

    if (!node.left) {
        LogWarning("Ошибка: левый операнд равен nullptr");
        result = nullptr;
        return;
    }
    node.left->accept(*this);
    llvm::Value* left = getResult();
    
    if (!left) {
        LogWarning("Ошибка: не удалось получить значение левого операнда");
        result = nullptr;
        return;
    }
  
    if (!node.right) {
        LogWarning("Ошибка: правый операнд равен nullptr");
        result = nullptr;
        return;
    }
    node.right->accept(*this);
    llvm::Value* right = getResult();
    
    if (!right) {
        LogWarning("Ошибка: не удалось получить значение правого операнда");
        result = nullptr;
        return;
    }
    
    result = Expressions::handleBinaryOperation(context, node, left, right);
}

void ASTGen::visit(UnaryOpNode& node) {
    LogWarning("Обработка UnaryOpNode: " + node.op);

    if (!node.operand) {
        LogWarning("Ошибка: операнд равен nullptr");
        result = nullptr;
        return;
    }
    
    node.operand->accept(*this);
    llvm::Value* operand = getResult();
    
    if (!operand) {
        LogWarning("Ошибка: не удалось получить значение операнда");
        result = nullptr;
        return;
    }
    
    result = Expressions::handleUnaryOperation(context, node, operand);
}

void ASTGen::visit(IdentifierNode& node) {
    LogWarning("visit для IdentifierNode: " + node.name);

    // Базовая заглушка: ищем переменную в таблице символов
    if (context.NamedValues.find(node.name) != context.NamedValues.end()) {
        result = context.NamedValues[node.name];
        LogWarning("Найдена переменная " + node.name);
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
    LogWarning("visit для BreakNode");
    llvm::BasicBlock* loopEnd = context.getCurrentLoopEndBlock();
    if (loopEnd) {
        if (!context.Builder.GetInsertBlock()->getTerminator()) {
            context.Builder.CreateBr(loopEnd);
        } else {
            LogWarning("Break statement in a block that is already terminated.");
        }
    }
    result = nullptr; // break не производит значения
}

void ASTGen::visit(ContinueNode& node) {
    LogWarning("visit для ContinueNode");
    llvm::BasicBlock* loopCond = context.getCurrentLoopCondBlock();
    if (loopCond) {
        if (!context.Builder.GetInsertBlock()->getTerminator()) {
            context.Builder.CreateBr(loopCond);
        } else {
            LogWarning("Continue statement in a block that is already terminated.");
        }
    }
    result = nullptr; // continue не производит значения
}

void ASTGen::visit(AccessExpression& node) {
    LogWarning("visit не реализован для AccessExpression: " + node.memberName);
    result = nullptr;
}

void ASTGen::visit(ImportNode &node)
{
    /*
    Они уже убраны к хуям собачим из AST перед этапом семантики
            Смысла реализовывать здесь что то нет
    */
}

void ASTGen::visit(LambdaNode &node)
{
    LogWarning("visit не реализован для LambdaNode");
    result = nullptr;
}