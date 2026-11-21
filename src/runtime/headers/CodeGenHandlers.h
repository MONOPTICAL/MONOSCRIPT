#ifndef CODEGENHANDLERS_H
#define CODEGENHANDLERS_H

#include "CodeGenContext.h"
#include <map>

class ASTGen;

namespace Declarations {
    llvm::Value*                    handleSimpleAssignment(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType);
    llvm::Value*                    handleGlobalVariable(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType);
    // REPL: глобальные переменные, которые уже живут в JIT прошлых строк, имя -> символ. Пусто вне REPL
    extern std::map<std::string, std::string> replGlobals;
    extern std::string replPrefix; // Префикс символов новых глобальных переменных. Пустой: обычная программа
  //llvm::Value*                    handleFunctionDeclaration(CodeGenContext& context, const std::string& name, llvm::FunctionType* type);
    llvm::Value*                    handleSimpleReassignment(CodeGenContext& context, VariableReassignNode& node, llvm::Type* varType);
};

namespace Arrays {
    bool                            isLiteral(const std::shared_ptr<ASTNode>& expression);
    llvm::Value*                    emitValue(CodeGenContext& context, const std::shared_ptr<ASTNode>& expression, const std::shared_ptr<TypeNode>& type);
    llvm::Value*                    emitLiteral(CodeGenContext& context, const std::shared_ptr<ASTNode>& literal, const std::shared_ptr<TypeNode>& arrayType);
    llvm::Value*                    handleArrayInitialization(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType, const std::shared_ptr<TypeNode>& arrayType);
    // Адрес элемента, на который указывает цепочка a[i][j], и его тип
    llvm::Value*                    emitElementPtr(CodeGenContext& context, AccessExpression& root, llvm::Type*& elementType, bool forWrite = false);
};

namespace Maps {
    bool                            isMap(const std::shared_ptr<TypeNode>& type);
    llvm::Value*                    emitLiteral(CodeGenContext& context, const std::shared_ptr<ASTNode>& literal, const std::shared_ptr<TypeNode>& type);
    llvm::Value*                    emitKey(CodeGenContext& context, const std::shared_ptr<ASTNode>& expression, const std::shared_ptr<TypeNode>& mapType);
    // Адрес 8-байтовой ячейки значения: forWrite создаёт ключ, иначе его отсутствие останавливает программу
    llvm::Value*                    emitSlot(CodeGenContext& context, llvm::Value* map, llvm::Value* key, bool forWrite, int line);
    llvm::Value*                    emitLen(CodeGenContext& context, llvm::Value* map);
    llvm::Value*                    emitCopy(CodeGenContext& context, llvm::Value* map);
    llvm::Value*                    emitHas(CodeGenContext& context, llvm::Value* map, llvm::Value* key);
    void                            emitDel(CodeGenContext& context, llvm::Value* map, llvm::Value* key, int line);
    void                            emitFor(CodeGenContext& context, ASTGen& codeGen, ForNode& node);
};

namespace Objects {
    void                            registerStruct(CodeGenContext& context, StructNode& node);
    void                            emitConstructor(CodeGenContext& context, StructNode& node);
    llvm::Value*                    emitNew(CodeGenContext& context, const std::string& name);
    llvm::Value*                    emitCopy(CodeGenContext& context, llvm::Value* source, const std::shared_ptr<TypeNode>& type);
};

namespace Statements {
    llvm::Value*                    handleReturnStatement(CodeGenContext& context, ReturnNode& node);
};

namespace Expressions {
    llvm::Value*                    handleBinaryOperation(CodeGenContext& context, BinaryOpNode& node, llvm::Value* left, llvm::Value* right);
    llvm::Value*                    handleUnaryOperation(CodeGenContext& context, UnaryOpNode& node, llvm::Value* operand);
}

namespace TypeConversions {
    // sourceUnsigned и targetUnsigned: у LLVM знака нет, его знает только тип в AST (u8..u64)
    llvm::Value*                    convertValueToType(CodeGenContext& context, llvm::Value* value, llvm::Type* targetType, const std::string& name = "",
                                                       bool sourceUnsigned = false, bool targetUnsigned = false);
    // char и u8..u64 хранятся без знака: каст в целое расширяет нулями, в float идёт как uitofp. Вызывается там, где значение появляется
    llvm::Value*                    applyCharCast(CodeGenContext& context, ASTNode& node, llvm::Value* value);
    llvm::Value*                    applyImplicitCast(CodeGenContext& context, llvm::Value* value, std::shared_ptr<TypeNode> targetTypeNode, const std::string& name = "",
                                                      bool sourceUnsigned = false);
    // Запись значения узла source в ячейку типа target: знак берётся из типа узла после каста и из target
    llvm::Value*                    convertNodeValue(CodeGenContext& context, llvm::Value* value, llvm::Type* targetType,
                                                     const std::shared_ptr<ASTNode>& source, const std::shared_ptr<TypeNode>& target);
    // Тип узла без знака: передаётся в applyImplicitCast
    inline bool                     isUnsignedNode(const std::shared_ptr<ASTNode>& node) {
                                        return node && node->inferredType && isUnsignedTypeName(node->inferredType->toString());
                                    }
    llvm::Value*                    loadValueIfPointer(CodeGenContext& context, llvm::Value* value, const std::string& name = "");
}
#endif