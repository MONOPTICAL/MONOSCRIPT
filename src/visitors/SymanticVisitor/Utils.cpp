#include "../headers/SymanticVisitor.h"

void SymanticVisitor::LogError(const std::string &message)
{
    throw std::runtime_error("Semantic Error: " + message);
}

std::shared_ptr<TypeNode> SymanticVisitor::checkForIdentifier(std::shared_ptr<ASTNode>& node)
{
    if (!node) {
        LogError("Node is null");
    }

    if (auto idNode = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        auto it = contexts.back().variables.find(idNode->name);
        if (it == contexts.back().variables.end()) 
            LogError("Variable not found: " + idNode->name);

        auto varAssign = std::dynamic_pointer_cast<VariableAssignNode>(it->second);
        if (!varAssign) 
            LogError("!!!This doesn't need to happen!!!");

        if (varAssign->expression == nullptr) {
            if (!varAssign->inferredType)
                LogError("Variable '" + idNode->name + "' has no inferred type");
            return varAssign->inferredType;
        }

        if (!varAssign->expression->inferredType)
            LogError("Expression type is null for variable: " + idNode->name);

        return varAssign->expression->inferredType;
    }

    if (!node->inferredType)
        LogError("Node type is null");

    return node->inferredType;
}

void SymanticVisitor::debugContexts()
{
    for (const auto& context : contexts) {
        std::cout << "Context: " << context.currentFunctionName << "\n";
        std::cout << "Variables:\n";
        for (const auto& var : context.variables) {
            std::cout << "  " << var.first << ": ";
            if (var.second) {
                auto typePtr = std::dynamic_pointer_cast<VariableAssignNode>(var.second);
                if (typePtr) {
                    std::cout << typePtr->name << " - ";
                    auto inferredType = typePtr->inferredType;
                    if (inferredType) {
                        std::cout << inferredType->toString() << "\n";
                    } else {
                        std::cout << "<null inferred type> ";
                        std::cout << typePtr->type->toString() << "\n";
                    }
                } else {
                    std::cout << "<null variable>\n";
                }
            } else {
                std::cout << "<null node>\n";
            }
        }
        std::cout << "Functions:\n";
        try {
            if (context.functions.empty()) {
                std::cout << "  <no functions>\n";
            } else {
                for (const auto& func : context.functions) {
                    if (func.first.empty()) {
                        std::cout << "  <unnamed function>\n";
                    } else {
                        std::cout << "  " << func.first << "\n";
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cout << "  <error accessing functions: " << e.what() << ">\n";
        } catch (...) {
            std::cout << "  <unknown error accessing functions>\n";
        }
    }

    std::cout << "---------------------\n";
}