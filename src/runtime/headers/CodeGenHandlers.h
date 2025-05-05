#ifndef CODEGENHANDLERS_H
#define CODEGENHANDLERS_H

#include "CodeGenContext.h"

namespace Declarations {
    llvm::Value*                    handleSimpleAssignment(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType);
    llvm::Value*                    handleGlobalVariable(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType);
    llvm::Value*                    handleGlobalStringVariable(CodeGenContext &context, VariableAssignNode &node, llvm::Type *varType, std::shared_ptr<StringNode> strNode);
    llvm::Value*                    handleGlobalArrayVariable(CodeGenContext &context, VariableAssignNode &node, llvm::Type *varType, std::shared_ptr<BlockNode> blockExpr);
  //llvm::Value*                    handleFunctionDeclaration(CodeGenContext& context, const std::string& name, llvm::FunctionType* type);
    llvm::Value*                    handleSimpleReassignment(CodeGenContext& context, VariableReassignNode& node, llvm::Type* varType);
};

namespace Arrays {
    llvm::Value*                    handleArrayInitialization(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType, std::shared_ptr<BlockNode> blockExpr);
};

namespace Statements {
    llvm::Value*                    handleReturnStatement(CodeGenContext& context, ReturnNode& node);
};

namespace Expressions {
    llvm::Value*                    handleBinaryOperation(CodeGenContext& context, BinaryOpNode& node, llvm::Value* left, llvm::Value* right);
    llvm::Value*                    handleUnaryOperation(CodeGenContext& context, UnaryOpNode& node, llvm::Value* operand);
}

namespace TypeConversions {
    llvm::Value*                    convertValueToType(CodeGenContext& context, llvm::Value* value, llvm::Type* targetType, const std::string& name = "");
    llvm::Value*                    applyImplicitCast(CodeGenContext& context, llvm::Value* value, std::shared_ptr<TypeNode> targetTypeNode, const std::string& name = "");
    llvm::Value*                    loadValueIfPointer(CodeGenContext& context, llvm::Value* value, const std::string& name = "");
}
#endif