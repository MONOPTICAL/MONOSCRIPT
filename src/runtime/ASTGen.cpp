#include "headers/ASTVisitors.h"
#include "headers/CodeGenHandlers.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>

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
    llvm::Type* varType = context.getLLVMType(node.type);
    if (!varType) {
        LogWarning("Не удалось получить тип для переменной " + node.name);
        result = nullptr;
        return;
    }

    llvm::BasicBlock* currentBlock = context.Builder.GetInsertBlock();
    bool isGlobalContext = (currentBlock == nullptr);
    
    if (isGlobalContext) {
        Declarations::handleGlobalVariable(context, node, varType, node.isConst);
        return;
    }
    
    // Проверяем является ли выражение структурой данных (array или map)
    std::shared_ptr<BlockNode> blockExpr = std::dynamic_pointer_cast<BlockNode>(node.expression);
    if (blockExpr) {
        Arrays::handleArrayInitialization(context, node, varType, blockExpr);
        return;
    }
    
    // Обычное присваивание (не массив или массив с одним выражением)
    Declarations::handleSimpleAssignment(context, node, varType);
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
    LogWarning("visit для ReturnNode: вычисляю выражение и генерирую return");
    
    if (node.expression) {
        node.expression->accept(*this);
    }
    
    llvm::Function* currentFunction = context.Builder.GetInsertBlock()->getParent();
    if (!result) {
        result = context.Builder.CreateRetVoid();
    } else {
        result = context.Builder.CreateRet(result);
    }
}

void ASTGen::visit(CallNode& node) {
    LogWarning("visit не реализован для CallNode: " + node.callee);
    result = nullptr;
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
    result = context.Builder.CreateGlobalStringPtr(node.value, "str");
}

void ASTGen::visit(BooleanNode& node) {
    LogWarning("visit для BooleanNode: " + std::string(node.value ? "true" : "false"));
    result = llvm::ConstantInt::get(context.TheContext, llvm::APInt(1, node.value ? 1 : 0));
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

void ASTGen::visit(ClassNode& node) {
    LogWarning("visit не реализован для ClassNode: " + node.name);
    result = nullptr;
}