#pragma once
#include "../../parser/headers/AST.h"
#include <memory>
#include <string>

std::shared_ptr<ProgramNode> symanticParseModule(
    std::shared_ptr<ProgramNode> combinedAST,
    bool showSymantic
);