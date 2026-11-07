#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"
#include "../../errors/headers/ErrorEngine.h"

llvm::Function* declareUserFunction(CodeGenContext& context, FunctionNode& node);

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
            throw std::runtime_error("Codegen Error: could not generate a collection element");

        value = TypeConversions::loadValueIfPointer(context, value);
        value = TypeConversions::applyImplicitCast(context, value, expression->implicitCastTo);
        return TypeConversions::convertValueToType(context, value, context.getLLVMType(type, context.TheContext));
    }

    llvm::Value* emitLiteral(CodeGenContext& context, const std::shared_ptr<ASTNode>& literal, const std::shared_ptr<TypeNode>& arrayType)
    {
        if (Maps::isMap(arrayType))
            return Maps::emitLiteral(context, literal, arrayType);

        auto generic = std::dynamic_pointer_cast<GenericTypeNode>(arrayType);
        if (!generic || generic->baseName != "array")
            throw std::runtime_error("Codegen Error: array literal for type " + (arrayType ? arrayType->toString() : std::string("?")));

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
            throw std::runtime_error("Codegen Error: access through [] and . is not implemented for: " + root.baseName);
        llvm::Value* base = TypeConversions::loadValueIfPointer(context, codeGen.getResult());

        // Каждое звено говорит своей нотацией, как получено следующее. Следующее звено хранит индекс или имя поля и тип результата
        AccessExpression* link = &root;
        std::shared_ptr<TypeNode> currentType = root.expression->inferredType;
        llvm::Value* elementPtr = nullptr;
        bool isVoidCall = false;
        while (auto next = std::dynamic_pointer_cast<AccessExpression>(link->nextAccess)) {
            // Уже не последнее звено: то, что лежит по адресу, это следующий объект
            if (elementPtr)
                base = context.Builder.CreateLoad(context.Builder.getPtrTy(), elementPtr, "nested_object");

            if (!next->inferredType)
                throw std::runtime_error("Codegen Error: unknown type in access chain: " + root.baseName);

            elementType = context.getLLVMType(next->inferredType, context.TheContext);

            auto resultType = std::dynamic_pointer_cast<GenericTypeNode>(currentType);
            if (link->notation == "." && resultType && resultType->baseName == "result") {
                llvm::StructType* layout = context.getResultStruct(currentType);
                const std::string member = next->baseName;
                if (member == "value") {
                    // Значения нет: останавливаемся с исходной причиной ошибки
                    llvm::Value* ok = context.Builder.CreateLoad(context.Builder.getInt8Ty(), context.Builder.CreateStructGEP(layout, base, 0), "ok");
                    llvm::Function* function = context.Builder.GetInsertBlock()->getParent();
                    llvm::BasicBlock* failBlock = llvm::BasicBlock::Create(context.TheContext, "result.fail", function);
                    llvm::BasicBlock* okBlock = llvm::BasicBlock::Create(context.TheContext, "result.ok", function);
                    context.Builder.CreateCondBr(context.Builder.CreateICmpNE(ok, context.Builder.getInt8(0)), okBlock, failBlock);

                    context.Builder.SetInsertPoint(failBlock);
                    llvm::Value* error = context.Builder.CreateLoad(context.Builder.getPtrTy(), context.Builder.CreateStructGEP(layout, base, 2), "error");
                    llvm::Function* panic = context.getOrDeclareFunction("ms_panic_result",
                        llvm::FunctionType::get(context.Builder.getVoidTy(), {context.Builder.getPtrTy(), context.Builder.getInt32Ty()}, false));
                    panic->setDoesNotReturn();
                    context.Builder.CreateCall(panic, {error, context.Builder.getInt32(ErrorEngine::getInstance().displayLine(next->line))});
                    context.Builder.CreateUnreachable();

                    context.Builder.SetInsertPoint(okBlock);
                    elementPtr = context.Builder.CreateStructGEP(layout, base, 3, "value");
                } else {
                    // ok хранится байтом: чтение i1 берёт из него 0 или 1
                    const unsigned index = member == "ok" ? 0 : member == "code" ? 1 : 2;
                    elementPtr = context.Builder.CreateStructGEP(layout, base, index, member);
                }
            } else if (link->notation == "[]" && Maps::isMap(currentType)) {
                // Только последнее звено записи создаёт ключ: в m[k][0] = x ключ k должен уже быть
                const bool createKey = forWrite && !next->nextAccess;
                elementPtr = Maps::emitSlot(context, base, Maps::emitKey(context, next->expression, currentType), createKey, next->line);
            } else if (link->notation == "[]" && currentType && currentType->toString() == "string") {
                next->expression->accept(codeGen);
                llvm::Value* index = TypeConversions::loadValueIfPointer(context, codeGen.getResult());
                index = TypeConversions::applyImplicitCast(context, index, next->expression->implicitCastTo);
                llvm::Value* wideIndex = index->getType()->isIntegerTy(1)
                    ? context.Builder.CreateZExt(index, context.Builder.getInt64Ty(), "index")
                    : context.Builder.CreateSExt(index, context.Builder.getInt64Ty(), "index");
                elementPtr = context.Builder.CreateCall(context.getOrDeclareFunction("ms_str_at",
                    llvm::FunctionType::get(context.Builder.getPtrTy(), {context.Builder.getPtrTy(), context.Builder.getInt64Ty(), context.Builder.getInt32Ty()}, false)),
                    {base, wideIndex, context.Builder.getInt32(ErrorEngine::getInstance().displayLine(next->line))}, "char_ptr");
            } else if (link->notation == "[]") {
                next->expression->accept(codeGen);
                llvm::Value* index = TypeConversions::loadValueIfPointer(context, codeGen.getResult());
                index = TypeConversions::applyImplicitCast(context, index, next->expression->implicitCastTo);
                elementPtr = context.getArrayElementPtr(base, elementType, index, next->line);
            } else {
                auto layout = currentType ? context.Structs.find(currentType->toString()) : context.Structs.end();
                if (layout == context.Structs.end())
                    throw std::runtime_error("Codegen Error: . used on something that is not a struct: " + root.baseName);

                // Вызов метода: результат кладём во временную ячейку, чтобы цепочка шла дальше как обычно
                if (auto call = std::dynamic_pointer_cast<CallNode>(next->expression)) {
                    auto method = layout->second.methods.at(call->callee);
                    llvm::Function* function = declareUserFunction(context, *method);

                    std::vector<llvm::Value*> arguments{base};
                    for (size_t i = 0; i < call->arguments.size(); i++) {
                        arguments.push_back(emitValue(context, call->arguments[i], method->parameters[i + 1].first));
                    }

                    llvm::Value* returned = context.Builder.CreateCall(function, arguments);
                    if (elementType->isVoidTy()) {
                        elementPtr = nullptr;
                        isVoidCall = true;
                    } else {
                        elementPtr = context.createEntryAlloca(elementType, "method_result");
                        context.Builder.CreateStore(returned, elementPtr);
                    }

                    currentType = next->inferredType;
                    link = next.get();
                    continue;
                }

                elementPtr = context.Builder.CreateStructGEP(layout->second.type, base, layout->second.fieldIndex.at(next->baseName), next->baseName);
            }

            currentType = next->inferredType;
            link = next.get();
        }

        if (!elementPtr && !isVoidCall)
            throw std::runtime_error("Codegen Error: access through . is not implemented for: " + root.baseName);
        return elementPtr;
    }
}
