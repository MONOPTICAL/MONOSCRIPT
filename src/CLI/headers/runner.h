#pragma once
#include "../../parser/headers/AST.h"
#include <memory>
#include <string>

namespace llvm {
    class Module;
}

int executeModule(llvm::Module* module, std::string mainFunction = "main", bool offOptimization = false);
int runProgram(std::shared_ptr<ProgramNode> combinedAST, const std::string& currentFilePath, bool showAST, bool offOptimization = false);