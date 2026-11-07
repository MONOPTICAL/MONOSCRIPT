#pragma once
#include "../../parser/headers/AST.h"
#include <climits>
#include <memory>
#include <string>

namespace llvm {
    class Module;
}

namespace ui {
    class Spinner;
}

// Программа не запустилась. Отдельное значение, чтобы не путать с кодом, который вернула сама программа
constexpr int RUN_FAILED = INT_MIN;

int executeModule(llvm::Module* module, std::string mainFunction = "main", bool offOptimization = false, ui::Spinner* spinner = nullptr);
int runProgram(std::shared_ptr<ProgramNode> combinedAST, const std::string& currentFilePath, bool showAST, bool offOptimization = false);