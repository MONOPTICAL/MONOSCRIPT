#pragma once
#include "../../lexer/headers/Token.h"
#include "../../parser/headers/AST.h"
#include <vector>
#include <memory>
#include <string>

std::vector<std::vector<Token>> tokenizeSource(const std::string& sourceCode, bool showTokens);
std::shared_ptr<ProgramNode> parseAndLinkModules(
    const std::vector<std::vector<Token>>& tokens, 
    const std::string& inputFile, 
    bool showAST
);