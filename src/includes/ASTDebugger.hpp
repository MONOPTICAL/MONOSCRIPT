#include "../parser/headers/AST.h"
#include "iostream"

class ASTDebugger 
{
    public:
        static void debug(const std::shared_ptr<ASTNode>& node, int indent = 0) {
            if (!node) {
                printIndent(indent);
                std::cout << "(null node)\n";
                return;
            }
    
            if (auto prog = std::dynamic_pointer_cast<ProgramNode>(node)) {
                printIndent(indent); std::cout << "[Program] " << prog->moduleName << "\n";
                for (const auto& stmt : prog->body) debug(stmt, indent + 2);
            }
            else if (auto func = std::dynamic_pointer_cast<FunctionNode>(node)) { 
                printIndent(indent); std::cout << "[Function] " << func->name << "[" << func->associated << "]" << "(...) Return";
                if (func->inferredType) 
                    std::cout << " (inferred type: " << func->inferredType->toString() << ")\n";
                else 
                    debug(func->returnType, 1);

                if (func->labels.size() > 0) {
                    printIndent(indent + 2); std::cout << "[Labels]: ";
                    for (const auto& label : func->labels) std::cout << label << " ";
                    std::cout << "\n";
                }

                for (const auto& param : func->parameters) {
                    printIndent(indent + 2); std::cout << "Param: " << param.second << " "; 
                    debug(param.first, 1);
                }
                debug(func->body, indent + 2);
            }
            else if(auto ifNode = std::dynamic_pointer_cast<IfNode>(node))
            {
                printIndent(indent); std::cout << "[IF CONDITION]: \n";
                debug(ifNode->condition, indent + 2);
                printIndent(indent); std::cout << "[IF], Then Block: \n";
                debug(ifNode->thenBlock, indent + 2);
                printIndent(indent); std::cout << "[ELSE], Else Block: \n";
                debug(ifNode->elseBlock, indent + 2);
            }
            else if (auto lambda = std::dynamic_pointer_cast<LambdaNode>(node))
            {
                printIndent(indent); std::cout << "[Lambda], Return Type: " << lambda->returnType->toString() << "\n";
                for (const auto& param : lambda->parameters) {
                    printIndent(indent + 2); std::cout << "Param: " << param.second << " "; 
                    debug(param.first, 1);
                }
                printIndent(indent); std::cout << "[Lambda], Body: \n";
                debug(lambda->body, indent + 2);
            }
            else if (auto For = std::dynamic_pointer_cast<ForNode>(node))
            {
                printIndent(indent); std::cout << "[For], iter_var: " << For->varName << "(auto)\n";
                printIndent(indent); std::cout << "[For], Iterable: \n";
                debug(For->iterable, indent + 2);
                printIndent(indent); std::cout << "[For], Body: \n";
                debug(For->body, indent + 2);

            }
            else if (auto use = std::dynamic_pointer_cast<ImportNode>(node)) {
                printIndent(indent); 
                std::cout << "[Use], Paths:\n";
                for (const auto& entry : use->paths) {
                    printIndent(indent + 2);
                    // Print the path (vector<string>)
                    for (size_t i = 0; i < entry.first.size(); ++i) {
                        std::cout << entry.first[i];
                        if (i != entry.first.size() - 1) std::cout << ".";
                    }
                    // Print alias if present
                    if (!entry.second.empty()) {
                        std::cout << " as " << entry.second;
                    }
                    std::cout << "\n";
                }
            }
            else if (auto block = std::dynamic_pointer_cast<BlockNode>(node)) {
                printIndent(indent); std::cout << "[Block]:\n";
                for (const auto& stmt : block->statements) debug(stmt, indent + 2);
            }
            else if (auto memberReassign = std::dynamic_pointer_cast<ReassignMemberNode>(node))
            {
                printIndent(indent); std::cout << "[Reassign], identifier: \n";
                debug(memberReassign->accessExpression, indent+2);
                printIndent(indent); std::cout << "[Reassign], value: \n";
                debug(memberReassign->expression, indent+2);
            }
            else if (auto assign = std::dynamic_pointer_cast<VariableAssignNode>(node)) {
                printIndent(indent); std::cout << "[Assign], IsConst: " << (assign->isConst ? "Const" : "Regular") << ", Identifier: " << assign->name << ",";
                debug(assign->type, 1);
                printIndent(indent); std::cout << "[Assign], value: \n";
                debug(assign->expression, indent + 2);
            }
            else if (auto reassign = std::dynamic_pointer_cast<VariableReassignNode>(node))
            {
                printIndent(indent); std::cout << "[Reassign], identifier: " << reassign->name << "\n";
                printIndent(indent); std::cout << "[Reassign], value: \n";
                debug(reassign->expression, indent + 2); 
            }
            else if (auto call = std::dynamic_pointer_cast<CallNode>(node)) {
                printIndent(indent); std::cout << "Call: " << call->callee << "(...)\n";
                for (const auto& arg : call->arguments) debug(arg, indent + 2);
            }
            else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
                printIndent(indent); std::cout << "BinaryOp: " << bin->op << "\n";
                debug(bin->left, indent + 2);
                debug(bin->right, indent + 2);
            }
            else if (auto access = std::dynamic_pointer_cast<AccessExpression>(node)) {
                printIndent(indent); std::cout << "[Access Notation]: " << access->notation << " Member: " << access->memberName << "\n";
                if(access->expression)
                    debug(access->expression, indent + 2);
                
                if(access->nextAccess)
                {
                    printIndent(indent+1); std::cout << "[Next Access]: \n";
                    debug(access->nextAccess, indent + 2);
                }
            }
            else if (auto un = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
                printIndent(indent); std::cout << "UnaryOp: " << un->op << "\n";
                debug(un->operand, indent + 2);
            }
            else if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node)) {
                printIndent(indent); std::cout << "Value: " << ident->name << " - <identifier>" << "\n";
                if (ident->implicitCastTo) {
                    printIndent(indent+2); std::cout << "[Implicit Cast To]: " << ident->implicitCastTo->toString() << "\n";
                }
            }
            else if (auto structNode = std::dynamic_pointer_cast<StructNode>(node))
            {
                printIndent(indent); std::cout << "[Struct], name: " << structNode->name << "\n";
                debug(structNode->body, indent+2);
            }
            else if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
                printIndent(indent); std::cout << "Value: " << num->value << " - <" + num->type->toString() + ">" << "\n";
                if (num->implicitCastTo) {
                    printIndent(indent+2); std::cout << "[Implicit Cast To]: " << num->implicitCastTo->toString() << "\n";
                }
            }
            else if (auto str = std::dynamic_pointer_cast<StringNode>(node)) {
                printIndent(indent); std::cout << "Value: " << str->value << " - <string>" << "\n";
                if (str->implicitCastTo) {
                    printIndent(indent+2); std::cout << "[Implicit Cast To]: " << str->implicitCastTo->toString() << "\n";
                }
            }
            else if (auto floatNum = std::dynamic_pointer_cast<FloatNumberNode>(node)) {
                printIndent(indent); std::cout << "Value: " << floatNum->value << " - <float>" << "\n";
                if (floatNum->implicitCastTo) {
                    printIndent(indent+2); std::cout << "[Implicit Cast To]: " << floatNum->implicitCastTo->toString() << "\n";
                }
            }
            else if (auto null = std::dynamic_pointer_cast<NullNode>(node)) {
                printIndent(indent); std::cout << "Value: <null>" << "\n";
            }
            else if (auto none = std::dynamic_pointer_cast<NoneNode>(node)){
                printIndent(indent); std::cout << "Value: <none>" << "\n";
            }
            else if(auto retrn = std::dynamic_pointer_cast<ReturnNode>(node)) {
                printIndent(indent); std::cout << "[Return]: " << "\n";
                debug(retrn->expression, indent+2);
            }
            else if(auto breakNode = std::dynamic_pointer_cast<BreakNode>(node)) {
                printIndent(indent); std::cout << "[Break]: Break Statement" << "\n";
            }
            else if(auto keyValue = std::dynamic_pointer_cast<KeyValueNode>(node)){
                printIndent(indent); std::cout << "Key: " << "\n";
                debug(keyValue->key, indent+2);
                printIndent(indent); std::cout << "Value: " << "\n";
                debug(keyValue->value, indent+2);
            }
            else if(auto whileNode = std::dynamic_pointer_cast<WhileNode>(node)) {
                printIndent(indent); std::cout << "[While], Condition: \n";
                debug(whileNode->condition, indent+2);
                printIndent(indent); std::cout << "[While], Body: \n";
                debug(whileNode->body, indent+2);
            }
            else if(auto typeShi = std::dynamic_pointer_cast<TypeNode>(node))
            {
                printIndent(indent);
                if (typeShi->inferredType)
                    std::cout << "Inferred Type: " << typeShi->toString() << "\n";
                else 
                    std::cout << "Type: " << typeShi->toString() << "\n";
            }
            else {
                printIndent(indent); std::cout << "Unknown AST node\n";
            }
        }
    
    private:
        static void printIndent(int level) {
            for (int i = 0; i < level; ++i) std::cout << ' ';
        }
};
//30982