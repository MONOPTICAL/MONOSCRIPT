#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"
namespace Declarations 
{
    // Значение объявления без значения: ноль, пустая строка. Объект получит значение до чтения, это проверила семантика
    static llvm::Value* emitDefaultValue(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType)
    {
        auto declared = node.inferredType ? node.inferredType : node.type;
        if (declared && declared->toString() == "string")
            return context.Builder.CreateGlobalString("", "empty_string");
        return llvm::Constant::getNullValue(varType);
    }

    static bool hasNoValue(const VariableAssignNode& node)
    {
        auto none = std::dynamic_pointer_cast<NoneNode>(node.expression);
        return none && !none->isEmptyLiteral;
    }

    llvm::Value* handleSimpleAssignment(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType)
    {
        ASTGen codeGen(context);

        if (hasNoValue(node)) {
            llvm::AllocaInst* variable = context.createEntryAlloca(varType, node.name);
            context.NamedValues[node.name] = variable;
            context.Builder.CreateStore(emitDefaultValue(context, node, varType), variable);
            return variable;
        }

        // Генерируем код для выражения
        node.expression->accept(codeGen);
        llvm::Value* value = codeGen.getResult();

        std::shared_ptr<TypeNode> typeNode;
        if (node.expression->inferredType)
            typeNode = node.expression->inferredType;
        else 
            typeNode = node.type;
        

        llvm::Type* valueType = value->getType();
        
        if (!value) {
            codeGen.LogWarning("Не удалось сгенерировать код для выражения " + node.name);
            return nullptr;
        }

        value = TypeConversions::loadValueIfPointer(context, value);
        // Значение всегда приводим к типу переменной: иначе в ячейку запишется не столько байт, сколько в ней есть
        value = TypeConversions::convertValueToType(context, value, varType);

        // Создаем переменную в таблице символов
        llvm::AllocaInst* alloca = context.createEntryAlloca(varType, node.name);
        context.NamedValues[node.name] = alloca;

        // Записываем значение в переменную
        context.Builder.CreateStore(value, alloca);
        return alloca;
    }
    
    // Сборщик не видит память JIT, поэтому глобальную переменную со ссылкой показываем ему сами
    static void registerGCRoot(CodeGenContext& context, llvm::GlobalVariable* globalVar)
    {
        llvm::Type* ptrType = llvm::PointerType::get(context.TheContext, 0);
        llvm::Function* addRoots = context.getOrDeclareFunction("GC_add_roots",
            llvm::FunctionType::get(llvm::Type::getVoidTy(context.TheContext), {ptrType, ptrType}, false)
        );

        const uint64_t size = context.TheModule->getDataLayout().getTypeAllocSize(globalVar->getValueType());
        llvm::Value* end = context.Builder.CreateConstGEP1_64(llvm::Type::getInt8Ty(context.TheContext), globalVar, size);
        context.Builder.CreateCall(addRoots, {globalVar, end});
    }

    // Вызывается внутри стартовой функции: всё, что не константа, вычисляется при запуске программы
    llvm::Value* handleGlobalVariable(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType)
    {
        ASTGen codeGen(context);

        llvm::Value* initValue = nullptr;
        if (hasNoValue(node)) {
            initValue = emitDefaultValue(context, node, varType);
        } else if (Arrays::isLiteral(node.expression)) {
            initValue = Arrays::emitLiteral(context, node.expression, node.inferredType ? node.inferredType : node.type);
        } else {
            node.expression->accept(codeGen);
            initValue = codeGen.getResult();
        }

        if (!initValue) {
            codeGen.LogWarning("Не удалось сгенерировать значение для глобальной переменной");
            return nullptr;
        }

        initValue = TypeConversions::loadValueIfPointer(context, initValue);
        initValue = TypeConversions::convertValueToType(context, initValue, varType);

        llvm::Constant* constantValue = llvm::dyn_cast<llvm::Constant>(initValue);

        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            *context.TheModule,
            varType,
            node.isConst && constantValue, // Переменную, в которую пишем при запуске, нельзя класть в константную память
            llvm::GlobalValue::PrivateLinkage,
            constantValue ? constantValue : llvm::Constant::getNullValue(varType),
            node.name
        );

        // Константная ссылка указывает на данные программы, сборщику там делать нечего
        if (varType->isPointerTy() && !globalVar->isConstant())
            registerGCRoot(context, globalVar);

        if (!constantValue)
            context.Builder.CreateStore(initValue, globalVar);

        context.NamedValues[node.name] = globalVar;
        return globalVar;
    }
}

llvm::Value *Declarations::handleSimpleReassignment(CodeGenContext &context, VariableReassignNode &node, llvm::Type *varType)
{
    ASTGen codeGen(context);
    llvm::Value* value = nullptr;
    if (Arrays::isLiteral(node.expression)) {
        value = Arrays::emitLiteral(context, node.expression, node.inferredType);
    } else {
        // Генерируем код для выражения
        node.expression->accept(codeGen);
        value = codeGen.getResult();
    }
    if (!value) {
        return nullptr;
    }

    value = TypeConversions::loadValueIfPointer(context, value);
    value = TypeConversions::convertValueToType(context, value, varType);

    // Записываем значение в переменную
    context.Builder.CreateStore(value, context.NamedValues[node.name]);
    return context.NamedValues[node.name];
}
