#include "headers/symantic.h"
#include "../visitors/headers/TypeSymbolVisitor.h"
#include "../includes/ASTDebugger.hpp"

std::shared_ptr<ProgramNode> symanticParseModule(std::shared_ptr<ProgramNode> combinedAST, bool showSymantic)
{
    // Type checking
    TypeSymbolVisitor typeSymbolVisitor;
    combinedAST->accept(typeSymbolVisitor);

    if (showSymantic && combinedAST) {
        std::cout << "\n--- AST(2) ---\n";
        ASTDebugger::debug(combinedAST);
        std::cout << "--- END AST(2) ---\n";
    }

    return combinedAST;
}