#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"

namespace Arrays
{
    bool isLiteral(const std::shared_ptr<ASTNode>& expression)
    {
        if (std::dynamic_pointer_cast<BlockNode>(expression)) return true;
        auto none = std::dynamic_pointer_cast<NoneNode>(expression);
        return none && none->isEmptyLiteral;
    }

    // Значение выражения, приведённое к нужному типу
    llvm::Value* emitValue(CodeGenContext& context, const std::shared_ptr<ASTNode>& expression, const std::shared_ptr<TypeNode>& type)
    {
        if (isLiteral(expression))
            return emitLiteral(context, expression, type);

        ASTGen codeGen(context);
        expression->accept(codeGen);
        llvm::Value* value = codeGen.getResult();
        if (!value)
            throw std::runtime_error("Codegen Error: не удалось сгенерировать элемент массива");

        value = TypeConversions::loadValueIfPointer(context, value);
        return TypeConversions::convertValueToType(context, value, context.getLLVMType(type, context.TheContext));
    }

    llvm::Value* emitLiteral(CodeGenContext& context, const std::shared_ptr<ASTNode>& literal, const std::shared_ptr<TypeNode>& arrayType)
    {
        if (Maps::isMap(arrayType))
            return Maps::emitLiteral(context, literal, arrayType);

        auto generic = std::dynamic_pointer_cast<GenericTypeNode>(arrayType);
        if (!generic || generic->baseName != "array")
            throw std::runtime_error("Codegen Error: литерал массива для типа " + (arrayType ? arrayType->toString() : std::string("?")));

        const std::shared_ptr<TypeNode>& elementTypeNode = generic->typeParameters[0];
        llvm::Type* elementType = context.getLLVMType(elementTypeNode, context.TheContext);

        // Пустой литерал это NoneNode, у него нет элементов
        static const std::vector<std::shared_ptr<ASTNode>> noElements;
        auto block = std::dynamic_pointer_cast<BlockNode>(literal);
        const auto& elements = block ? block->statements : noElements;

        llvm::Value* array = context.createArray(elementType, context.Builder.getInt32(elements.size()));
        llvm::Value* data = context.getArrayData(array);

        for (size_t i = 0; i < elements.size(); i++) {
            llvm::Value* value = emitValue(context, elements[i], elementTypeNode);
            context.Builder.CreateStore(value, context.Builder.CreateConstGEP1_64(elementType, data, i, "element_ptr"));
        }

        return array;
    }

    llvm::Value* handleArrayInitialization(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType, const std::shared_ptr<TypeNode>& arrayType)
    {
        llvm::Value* array = emitLiteral(context, node.expression, arrayType);

        llvm::AllocaInst* variable = context.createEntryAlloca(varType, node.name);
        context.Builder.CreateStore(array, variable);
        context.NamedValues[node.name] = variable;
        return variable;
    }

    llvm::Value* emitElementPtr(CodeGenContext& context, AccessExpression& root, llvm::Type*& elementType, bool forWrite)
    {
        ASTGen codeGen(context);
        root.expression->accept(codeGen);
        if (!codeGen.getResult())
            throw std::runtime_error("Codegen Error: доступ через [] и . пока не реализован: " + root.baseName);
        llvm::Value* base = TypeConversions::loadValueIfPointer(context, codeGen.getResult());

        // Каждое звено говорит своей нотацией, как получено следующее. Следующее звено хранит индекс или имя поля и тип результата
        AccessExpression* link = &root;
        std::shared_ptr<TypeNode> currentType = root.expression->inferredType;
        llvm::Value* elementPtr = nullptr;
        while (auto next = std::dynamic_pointer_cast<AccessExpression>(link->nextAccess)) {
            // Уже не последнее звено: то, что лежит по адресу, это следующий объект
            if (elementPtr)
                base = context.Builder.CreateLoad(context.Builder.getPtrTy(), elementPtr, "nested_object");

            if (!next->inferredType)
                throw std::runtime_error("Codegen Error: не удалось определить тип в цепочке доступа: " + root.baseName);

            elementType = context.getLLVMType(next->inferredType, context.TheContext);

            if (link->notation == "[]" && Maps::isMap(currentType)) {
                // Только последнее звено записи создаёт ключ: в m[k][0] = x ключ k должен уже быть
                const bool createKey = forWrite && !next->nextAccess;
                elementPtr = Maps::emitSlot(context, base, Maps::emitKey(context, next->expression, currentType), createKey, next->line);
            } else if (link->notation == "[]") {
                next->expression->accept(codeGen);
                llvm::Value* index = TypeConversions::loadValueIfPointer(context, codeGen.getResult());
                index = TypeConversions::applyImplicitCast(context, index, next->expression->implicitCastTo);
                elementPtr = context.getArrayElementPtr(base, elementType, index, next->line);
            } else {
                auto layout = currentType ? context.Structs.find(currentType->toString()) : context.Structs.end();
                if (layout == context.Structs.end())
                    throw std::runtime_error("Codegen Error: доступ через . не к структуре: " + root.baseName);
                elementPtr = context.Builder.CreateStructGEP(layout->second.type, base, layout->second.fieldIndex.at(next->baseName), next->baseName);
            }

            currentType = next->inferredType;
            link = next.get();
        }

        if (!elementPtr)
            throw std::runtime_error("Codegen Error: доступ через . пока не реализован: " + root.baseName);
        return elementPtr;
    }
}
