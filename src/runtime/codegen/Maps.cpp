#include "../headers/CodeGenHandlers.h"
#include "../headers/ASTVisitors.h"
#include "../../errors/headers/ErrorEngine.h"

namespace Maps
{
    static llvm::Type* ptrType(CodeGenContext& context) { return llvm::PointerType::get(context.TheContext, 0); }

    static llvm::Function* runtime(CodeGenContext& context, const std::string& name, llvm::Type* result, std::vector<llvm::Type*> arguments)
    {
        return context.getOrDeclareFunction(name, llvm::FunctionType::get(result, arguments, false));
    }

    static llvm::Value* sourceLine(CodeGenContext& context, int line)
    {
        return context.Builder.getInt32(ErrorEngine::getInstance().displayLine(line));
    }

    static std::shared_ptr<GenericTypeNode> mapType(const std::shared_ptr<TypeNode>& type)
    {
        auto generic = std::dynamic_pointer_cast<GenericTypeNode>(type);
        return generic && generic->baseName == "map" ? generic : nullptr;
    }

    bool isMap(const std::shared_ptr<TypeNode>& type)
    {
        return mapType(type) != nullptr;
    }

    // Ключ в 8-байтовой ячейке: строка это адрес, целое расширяется со знаком, i1 это 0 или 1
    llvm::Value* emitKey(CodeGenContext& context, const std::shared_ptr<ASTNode>& expression, const std::shared_ptr<TypeNode>& mapTypeNode)
    {
        const auto& keyType = mapType(mapTypeNode)->typeParameters[0];
        llvm::Value* key = Arrays::emitValue(context, expression, keyType);

        llvm::Type* i64Type = context.Builder.getInt64Ty();
        if (key->getType()->isPointerTy())
            return context.Builder.CreatePtrToInt(key, i64Type, "map_key");
        if (key->getType()->isIntegerTy(1))
            return context.Builder.CreateZExt(key, i64Type, "map_key");
        return context.Builder.CreateSExt(key, i64Type, "map_key");
    }

    llvm::Value* emitNew(CodeGenContext& context, const std::shared_ptr<TypeNode>& type)
    {
        auto generic = mapType(type);
        llvm::Type* i32Type = context.Builder.getInt32Ty();
        const bool keyIsString = generic->typeParameters[0]->toString() == "string";
        const bool valueIsReference = context.getLLVMType(generic->typeParameters[1], context.TheContext)->isPointerTy();

        return context.emitCall(
            runtime(context, "ms_map_new", ptrType(context), {i32Type, i32Type}),
            {context.Builder.getInt32(keyIsString), context.Builder.getInt32(keyIsString || valueIsReference)},
            "map"
        );
    }

    llvm::Value* emitLiteral(CodeGenContext& context, const std::shared_ptr<ASTNode>& literal, const std::shared_ptr<TypeNode>& type)
    {
        auto generic = mapType(type);
        llvm::Value* map = emitNew(context, type);

        // Пустой литерал это NoneNode, у него нет пар
        auto block = std::dynamic_pointer_cast<BlockNode>(literal);
        if (!block) return map;

        for (const auto& statement : block->statements) {
            auto pair = std::dynamic_pointer_cast<KeyValueNode>(statement);
            if (!pair)
                throw std::runtime_error("Codegen Error: a map literal expects key-value pairs");

            llvm::Value* key = emitKey(context, pair->key, type);
            llvm::Value* value = Arrays::emitValue(context, pair->value, generic->typeParameters[1]);
            context.Builder.CreateStore(value, emitSlot(context, map, key, true, pair->line));
        }
        return map;
    }

    llvm::Value* emitSlot(CodeGenContext& context, llvm::Value* map, llvm::Value* key, bool forWrite, int line)
    {
        llvm::Type* i64Type = context.Builder.getInt64Ty();
        if (forWrite)
            return context.emitCall(
                runtime(context, "ms_map_insert", ptrType(context), {ptrType(context), i64Type}), {map, key}, "map_slot");

        return context.emitCall(
            runtime(context, "ms_map_get", ptrType(context), {ptrType(context), i64Type, context.Builder.getInt32Ty()}),
            {map, key, sourceLine(context, line)}, "map_slot");
    }

    llvm::Value* emitLen(CodeGenContext& context, llvm::Value* map)
    {
        return context.emitCall(runtime(context, "ms_map_len", context.Builder.getInt32Ty(), {ptrType(context)}), {map}, "map_len");
    }

    llvm::Value* emitCopy(CodeGenContext& context, llvm::Value* map)
    {
        return context.emitCall(runtime(context, "ms_map_copy", ptrType(context), {ptrType(context)}), {map}, "map_copy");
    }

    llvm::Value* emitHas(CodeGenContext& context, llvm::Value* map, llvm::Value* key)
    {
        llvm::Value* found = context.emitCall(
            runtime(context, "ms_map_has", context.Builder.getInt32Ty(), {ptrType(context), context.Builder.getInt64Ty()}), {map, key}, "map_has");
        return context.Builder.CreateICmpNE(found, context.Builder.getInt32(0), "map_found");
    }

    void emitDel(CodeGenContext& context, llvm::Value* map, llvm::Value* key, int line)
    {
        context.emitCall(
            runtime(context, "ms_map_del", context.Builder.getVoidTy(), {ptrType(context), context.Builder.getInt64Ty(), context.Builder.getInt32Ty()}),
            {map, key, sourceLine(context, line)});
    }

    // for k in m. Набор ключей проверяет рантайм: при добавлении пары могут переехать
    void emitFor(CodeGenContext& context, ASTGen& codeGen, ForNode& node)
    {
        llvm::Type* i32Type = context.Builder.getInt32Ty();
        llvm::Type* i64Type = context.Builder.getInt64Ty();
        llvm::Type* keyType = context.getLLVMType(node.varType, context.TheContext);

        node.iterable->accept(codeGen);
        llvm::Value* map = TypeConversions::loadValueIfPointer(context, codeGen.getResult(), "for.map");
        llvm::Value* version = context.emitCall(runtime(context, "ms_map_version", i32Type, {ptrType(context)}), {map}, "for.version");

        llvm::AllocaInst* position = context.createEntryAlloca(i32Type, "for.position");
        llvm::AllocaInst* keySlot = context.createEntryAlloca(i64Type, "for.key");
        llvm::AllocaInst* variable = context.createEntryAlloca(keyType, node.varName);
        context.Builder.CreateStore(context.Builder.getInt32(0), position);

        llvm::Function* function = context.Builder.GetInsertBlock()->getParent();
        llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context.TheContext, "for.cond", function);
        llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(context.TheContext, "for.body", function);
        llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(context.TheContext, "for.end", function);

        context.pushLoopContext(condBlock, endBlock);
        context.Builder.CreateBr(condBlock);

        context.Builder.SetInsertPoint(condBlock);
        llvm::Value* hasNext = context.emitCall(
            runtime(context, "ms_map_next", i32Type, {ptrType(context), ptrType(context), i32Type, i32Type, ptrType(context)}),
            {map, position, version, sourceLine(context, node.line), keySlot}, "for.next");
        context.Builder.CreateCondBr(context.Builder.CreateICmpNE(hasNext, context.Builder.getInt32(0)), bodyBlock, endBlock);

        auto outerNamedValues = context.NamedValues;
        context.NamedValues[node.varName] = variable;

        context.Builder.SetInsertPoint(bodyBlock);
        context.Builder.CreateStore(context.Builder.CreateLoad(keyType, keySlot, "for.key_value"), variable);
        node.body->accept(codeGen);
        if (!context.Builder.GetInsertBlock()->getTerminator())
            context.Builder.CreateBr(condBlock);

        context.NamedValues = outerNamedValues;

        context.Builder.SetInsertPoint(endBlock);
        context.popLoopContext();
    }
}
