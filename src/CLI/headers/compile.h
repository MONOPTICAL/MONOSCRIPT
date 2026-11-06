#pragma once
#include <memory>
#include <string>
#include "../../parser/headers/AST.h"

bool compileToExecutable(std::shared_ptr<ProgramNode> combinedAST, const std::string& outputFile);