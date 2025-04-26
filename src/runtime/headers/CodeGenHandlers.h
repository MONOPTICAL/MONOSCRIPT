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
#endif