#pragma once
#include "../../parser/headers/AST.h"
#include <climits>
#include <memory>
#include <string>

namespace llvm {
    class Module;
    class LLVMContext;
}

// Программа не запустилась. Отдельное значение, чтобы не путать с кодом, который вернула сама программа
constexpr int RUN_FAILED = INT_MIN;

int executeModule(std::unique_ptr<llvm::Module> module, std::unique_ptr<llvm::LLVMContext> context, std::string mainFunction, bool offOptimization);
int runProgram(std::shared_ptr<ProgramNode> combinedAST, const std::string& currentFilePath, bool showAST, bool offOptimization = false);