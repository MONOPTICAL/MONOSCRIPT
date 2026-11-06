#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"

namespace Objects
{
    static std::shared_ptr<TypeNode> fieldType(const VariableAssignNode& field)
    {
        return field.inferredType ? field.inferredType : field.type;
    }

    // Раскладка нужна до генерации функций: P() можно вызвать раньше объявления структуры
    void registerStruct(CodeGenContext& context, StructNode& node)
    {
        StructLayout layout;
        layout.node = &node;

        std::vector<llvm::Type*> fieldTypes;
        if (auto body = std::dynamic_pointer_cast<BlockNode>(node.body)) {
            for (const auto& statement : body->statements) {
                auto field = std::dynamic_pointer_cast<VariableAssignNode>(statement);
                if (!field) continue; // Методы отложены

                llvm::Type* type = context.getLLVMType(fieldType(*field), context.TheContext);
                layout.fieldIndex[field->name] = fieldTypes.size();
                layout.fields.push_back(field);
                layout.hasReferences = layout.hasReferences || type->isPointerTy();
                fieldTypes.push_back(type);
            }
        }
        layout.type = llvm::StructType::create(context.TheContext, fieldTypes, "ms_struct_" + node.name);

        layout.constructor = llvm::Function::Create(
            llvm::FunctionType::get(llvm::PointerType::get(context.TheContext, 0), false),
            llvm::Function::PrivateLinkage, "__ms_new_" + node.name, context.TheModule.get()
        );

        context.Structs[node.name] = layout;
    }

    llvm::Value* allocate(CodeGenContext& context, const StructLayout& layout)
    {
        const uint64_t size = context.TheModule->getDataLayout().getTypeAllocSize(layout.type);
        return context.allocateObject(context.Builder.getInt64(size), layout.hasReferences, "object");
    }

    // Значения по умолчанию вычисляются при каждом P(), поэтому конструктор это функция
    void emitConstructor(CodeGenContext& context, StructNode& node)
    {
        const StructLayout& layout = context.Structs.at(node.name);

        auto outerNamedValues = context.NamedValues;
        context.NamedValues.clear();

        context.Builder.SetInsertPoint(llvm::BasicBlock::Create(context.TheContext, "entry", layout.constructor));
        llvm::Value* object = allocate(context, layout);

        for (size_t i = 0; i < layout.fields.size(); i++) {
            llvm::Value* value = Arrays::emitValue(context, layout.fields[i]->expression, fieldType(*layout.fields[i]));
            context.Builder.CreateStore(value, context.Builder.CreateStructGEP(layout.type, object, i));
        }
        context.Builder.CreateRet(object);

        context.Builder.ClearInsertionPoint();
        context.NamedValues = outerNamedValues;
    }

    llvm::Value* emitNew(CodeGenContext& context, const std::string& name)
    {
        return context.Builder.CreateCall(context.Structs.at(name).constructor, {}, "object");
    }

    // Поверхностная копия: вложенные объекты остаются общими
    llvm::Value* emitCopy(CodeGenContext& context, llvm::Value* source, const std::shared_ptr<TypeNode>& type)
    {
        const llvm::Align align(8);

        auto generic = std::dynamic_pointer_cast<GenericTypeNode>(type);
        if (generic && generic->baseName == "map")
            throw std::runtime_error("Codegen Error: copy для map пока не реализован");

        if (generic) {
            llvm::Type* elementType = context.getLLVMType(generic->typeParameters[0], context.TheContext);
            llvm::Value* length = context.getArrayLength(source);
            llvm::Value* result = context.createArray(elementType, length);
            llvm::Value* bytes = context.Builder.CreateMul(
                context.Builder.CreateZExt(length, context.Builder.getInt64Ty()),
                context.Builder.getInt64(context.TheModule->getDataLayout().getTypeAllocSize(elementType))
            );
            context.Builder.CreateMemCpy(context.getArrayData(result), align, context.getArrayData(source), align, bytes);
            return result;
        }

        const StructLayout& layout = context.Structs.at(type->toString());
        llvm::Value* result = allocate(context, layout);
        context.Builder.CreateMemCpy(result, align, source, align, context.TheModule->getDataLayout().getTypeAllocSize(layout.type));
        return result;
    }
}
