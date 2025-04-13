#ifndef CODEGENHANDLERS_H
#define CODEGENHANDLERS_H

#include "CodeGenContext.h"

namespace Declarations {
    llvm::Value* handleSimpleAssignment(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType);
    llvm::Value* handleGlobalVariable(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType, bool isConstant);
    //llvm::Value* handleFunctionDeclaration(CodeGenContext& context, const std::string& name, llvm::FunctionType* type);
};

namespace Arrays {
    llvm::Value* handleArrayInitialization(CodeGenContext& context, VariableAssignNode& node, llvm::Type* varType, std::shared_ptr<BlockNode> blockExpr);
};

#endif