#include "lexer/headers/Lexer.h"
#include "lexer/headers/Token.h"
#include "parser/headers/Parser.h"
#include "iostream"


class ASTDebugger {
    public:
        static void debug(const std::shared_ptr<ASTNode>& node, int indent = 0) {
            if (!node) {
                printIndent(indent);
                std::cout << "(null node)\n";
                return;
            }
    
            if (auto prog = std::dynamic_pointer_cast<ProgramNode>(node)) {
                printIndent(indent); std::cout << "ProgramNode\n";
                for (const auto& stmt : prog->body) debug(stmt, indent + 2);
            }
            else if (auto func = std::dynamic_pointer_cast<FunctionNode>(node)) {
                printIndent(indent); std::cout << "Function: " << func->returnType << " " << func->name << "(...)\n";
                for (const auto& param : func->parameters) {
                    printIndent(indent + 2); std::cout << "Param: " << param.first << " " << param.second << "\n";
                }
                debug(func->body, indent + 2);
            }
            else if(auto ifNode = std::dynamic_pointer_cast<IfNode>(node))
            {
                printIndent(indent); std::cout << "IF condition: \n";
                debug(ifNode->condition, indent + 2);
                printIndent(indent); std::cout << "Then Block: \n";
                debug(ifNode->thenBlock, indent + 2);
                printIndent(indent); std::cout << "Else Block: \n";
                debug(ifNode->elseBlock, indent + 2);
            }
            else if (auto For = std::dynamic_pointer_cast<ForNode>(node))
            {
                printIndent(indent); std::cout << "For, iter_var: " << For->varName << "(auto)\n";
                printIndent(indent); std::cout << "Iterable: \n";
                debug(For->iterable, indent + 2);
                printIndent(indent); std::cout << "Body: \n";
                debug(For->body, indent + 2);

            }
            else if (auto block = std::dynamic_pointer_cast<BlockNode>(node)) {
                printIndent(indent); std::cout << "Block\n";
                for (const auto& stmt : block->statements) debug(stmt, indent + 2);
            }
            else if (auto assign = std::dynamic_pointer_cast<VariableAssignNode>(node)) {
                printIndent(indent); std::cout << "Assign, type: " << (assign->isConst ? "Const " : "Final ") << assign->type << ", Identifier: " << assign->name << "\n";
                debug(assign->expression, indent + 2);
            }
            else if (auto reassign = std::dynamic_pointer_cast<VariableReassignNode>(node))
            {
                printIndent(indent); std::cout << "Reassign, identifier: " << reassign->name << "\n";
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
                printIndent(indent); std::cout << "Access Notation: " << access->notation << " Member: " << access->memberName << "\n";
                if(access->expression)
                    debug(access->expression, indent + 2);
                
                if(access->nextAccess)
                {
                    printIndent(indent); std::cout << "Next Access: \n";
                    debug(access->nextAccess, indent + 2);
                }
            }
            else if (auto un = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
                printIndent(indent); std::cout << "UnaryOp: " << un->op << "\n";
                debug(un->operand, indent + 2);
            }
            else if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node)) {
                printIndent(indent); std::cout << "Value: " << ident->name << " - <identifier>" << "\n";
            }
            else if (auto structNode = std::dynamic_pointer_cast<StructNode>(node))
            {
                printIndent(indent); std::cout << "Struct, name: " << structNode->name << "\n";
                debug(structNode->body, indent+2);
            }
            else if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
                printIndent(indent); std::cout << "Value: " << num->value << " - <i32/i64>" << "\n";
            }
            else if (auto str = std::dynamic_pointer_cast<StringNode>(node)) {
                printIndent(indent); std::cout << "Value: " << str->value << " - <string>" << "\n";
            }
            else if (auto boolean = std::dynamic_pointer_cast<BooleanNode>(node)) {
                printIndent(indent); std::cout << "Value: " << (boolean->value ? "true" : "false") << " - <bool>" << "\n";
            }
            else if (auto floatNum = std::dynamic_pointer_cast<FloatNumberNode>(node)) {
                printIndent(indent); std::cout << "Value: " << floatNum->value << " - <float>" << "\n";
            }
            else if (auto null = std::dynamic_pointer_cast<NullNode>(node)) {
                printIndent(indent); std::cout << "Value: <null>" << "\n";
            }
            else if (auto none = std::dynamic_pointer_cast<NoneNode>(node)){
                printIndent(indent); std::cout << "Value: <none>" << "\n";
            }
            else if(auto retrn = std::dynamic_pointer_cast<ReturnNode>(node)) {
                printIndent(indent); std::cout << "Return: " << "\n";
                debug(retrn->expression, indent+2);
            }
            else if(auto breakNode = std::dynamic_pointer_cast<BreakNode>(node)) {
                printIndent(indent); std::cout << "Break: Break Statement" << "\n";
            }
            else if(auto keyValue = std::dynamic_pointer_cast<KeyValueNode>(node)){
                printIndent(indent); std::cout << "Key: " << "\n";
                debug(keyValue->key, indent+2);
                printIndent(indent); std::cout << "Value: " << "\n";
                debug(keyValue->value, indent+2);
            }
            else if(auto whileNode = std::dynamic_pointer_cast<WhileNode>(node)) {
                printIndent(indent); std::cout << "While, Condition: \n";
                debug(whileNode->condition, indent+2);
                printIndent(indent); std::cout << "While, Body: \n";
                debug(whileNode->body, indent+2);
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

// g++ main.cpp lexer/Lexer.cpp lexer/ParsingFunctions.cpp parser/Logic.cpp parser/Parser.cpp parser/Parsers.cpp -o main -std=c++20
int main()
{
    std::string sourceCode;
    std::string line;

    std::cout << "Enter source code (end with Ctrl+Z on Windows or Ctrl+D on Linux/Mac):" << std::endl;

    // Считываем строки до конца ввода
    while (std::getline(std::cin, line))
    {
        sourceCode += line + "\n";
    }

    std::cout << "Source code:" << std::endl << sourceCode << std::endl;

    Lexer lexer(sourceCode);
    lexer.tokenize();
    const std::vector<std::vector<Token>> X = lexer.getTokens();

    Parser parser(X);
    std::shared_ptr<ProgramNode> program = parser.parse();
    if (program) {
        ASTDebugger::debug(program);
    } else {
        std::cerr << "Failed to parse the source code." << std::endl;
    }

    
    return 0;
}