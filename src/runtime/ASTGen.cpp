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
        paramTypes.push_back(context.getLLVMType(param.first));
    }
    
    llvm::Type* returnLLVMType = llvm::Type::getVoidTy(context.TheContext);
    if (node.returnType) {
        returnLLVMType = context.getLLVMType(node.returnType);
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
    llvm::Type* varType = context.getLLVMType(node.inferredType);

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
    result = nullptr;
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
    // --- Специальная обработка для встроенной функции echo ---
    if (node.callee == "echo") {
        LogWarning("visit для CallNode (echo)");

        // 1. Получаем или объявляем функцию printf
        llvm::FunctionType* printfType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(context.TheContext),
            {llvm::PointerType::get(llvm::Type::getInt8Ty(context.TheContext), 0)}, // Теперь должно работать с правильными include
            true
        );
        llvm::FunctionCallee printfFunc = context.TheModule->getOrInsertFunction("printf", printfType);

        // 2. Обрабатываем каждый аргумент для echo
        for (const auto& argNode : node.arguments) {
            if (!argNode) continue;

            argNode->accept(*this);
            llvm::Value* argValue = getResult();

            if (!argValue) {
                LogWarning("Не удалось сгенерировать значение для аргумента echo");
                continue;
            }

            llvm::Type* argType = argValue->getType();
            std::string formatStringValue;
            llvm::Value* valueToPrint = argValue;

            // 3. Определяем форматную строку и подготавливаем значение
            if (argType->isIntegerTy(32)) {
                formatStringValue = "%d";
            } else if (argType->isIntegerTy(64)) {
                formatStringValue = "%lld";
            } else if (argType->isFloatTy()) {
                formatStringValue = "%f";
                valueToPrint = context.Builder.CreateFPExt(argValue, llvm::Type::getDoubleTy(context.TheContext), "fpext");
            } else if (argType->isDoubleTy()) {
                 formatStringValue = "%f";
            } else if (argType->isIntegerTy(1)) {
                formatStringValue = "%d";
                valueToPrint = context.Builder.CreateZExt(argValue, llvm::Type::getInt32Ty(context.TheContext), "zext");
            } else if (argType->isPointerTy()) {
                // В LLVM 17+ мы не можем узнать тип элемента указателя!
                // Поэтому: если это строка (i8*), мы знаем это только по логике генерации!
                // Попробуем определить по имени переменной или другим признакам, иначе печатаем адрес
            
                // Если вы точно знаете, что это строка (например, результат visit(StringNode)), используйте %s:
                if (llvm::isa<llvm::ConstantExpr>(argValue) || llvm::isa<llvm::Constant>(argValue)) {
                    formatStringValue = "%s";
                } else {
                    // Для всех остальных указателей — только адрес
                    formatStringValue = "%p";
                    valueToPrint = context.Builder.CreateBitCast(
                        argValue,
                        llvm::PointerType::get(llvm::Type::getInt8Ty(context.TheContext), 0),
                        "ptr_cast"
                    );
                }
            } else {
                // --- Замена context.typeToString ---
                std::string typeStr;
                llvm::raw_string_ostream rso(typeStr);
                if (argType) {
                    argType->print(rso);
                } else {
                    rso << "<null type>";
                }
                LogWarning("Неподдерживаемый тип для echo: " + rso.str()); // Используем rso.str()
                // --- Конец замены ---

                formatStringValue = "[Неподдерживаемый тип: " + rso.str() + "]"; // Добавим тип в сообщение
                valueToPrint = context.Builder.CreateGlobalString(formatStringValue, "unsupported_str");
                formatStringValue = "%s";
            }

            formatStringValue += "\n";

            // 4. Создаем глобальную строку для формата
            llvm::Constant* formatString = context.Builder.CreateGlobalString(formatStringValue, "fmt");

            // 5. Генерируем вызов printf
            std::vector<llvm::Value*> printfArgs = {formatString, valueToPrint};
            context.Builder.CreateCall(printfFunc, printfArgs, "printfCall");
        }
        result = nullptr;
    }
    else {
        LogWarning("visit не реализован для CallNode: " + node.callee);
        llvm::Function* calleeFunc = context.TheModule->getFunction(node.callee);
        if (!calleeFunc) {
            LogWarning("Неизвестная функция: " + node.callee); // Используем LogWarning или LogError
            result = nullptr;
            return;
        }

        if (calleeFunc->arg_size() != node.arguments.size()) {
            LogWarning("Неверное количество аргументов для функции " + node.callee); // Используем LogWarning или LogError
            result = nullptr;
            return;
        }

        std::vector<llvm::Value*> argsV;
        for (unsigned i = 0, e = node.arguments.size(); i != e; ++i) {
            if (!node.arguments[i]) {
                LogWarning("Пустой узел аргумента для функции " + node.callee); // Используем LogWarning или LogError
                 result = nullptr;
                 return;
            }
            node.arguments[i]->accept(*this);
            llvm::Value* argVal = getResult();
            if (!argVal) {
                LogWarning("Не удалось сгенерировать аргумент " + std::to_string(i) + " для функции " + node.callee); // Используем LogWarning или LogError
                result = nullptr;
                return;
            }
            argsV.push_back(argVal);
        }
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
    result = llvm::ConstantInt::get(context.TheContext, llvm::APInt(32, node.value, true));
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