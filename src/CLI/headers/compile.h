#pragma once
#include <memory>
#include <string>
#include "../../parser/headers/AST.h"

void compileToExecutable(std::shared_ptr<ProgramNode> combinedAST, const std::string& outputFile);