#pragma once
#include "../../parser/headers/AST.h"
#include <climits>
#include <memory>
#include <string>

namespace llvm {
    class Module;
}

// Программа не запустилась. Отдельное значение, чтобы не путать с кодом, который вернула сама программа
constexpr int RUN_FAILED = INT_MIN;

int executeModule(llvm::Module* module, std::string mainFunction = "main", bool offOptimization = false);
int runProgram(std::shared_ptr<ProgramNode> combinedAST, const std::string& currentFilePath, bool showAST, bool offOptimization = false);