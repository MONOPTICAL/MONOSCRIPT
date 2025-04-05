#ifndef AST_H
#define AST_H
#include <memory>
#include <string>
#include <vector>

class ASTNode {
    public:
        int line; // номер строки в исходном коде
        int column; // номер столбца в исходном коде
        virtual ~ASTNode() = default;
};

class ProgramNode : public ASTNode {
    public:
        std::vector<std::shared_ptr<ASTNode>> body;
};
    
class FunctionNode : public ASTNode {
    public:
        std::shared_ptr<IdentifierNode> name; // Имя функции тоже идентификатор
        std::string returnType;
        std::vector<std::pair<std::string, std::string>> parameters; // {type, name}
        std::shared_ptr<ASTNode> body; // BlockNode
};

class BlockNode : public ASTNode {
    public:
        std::vector<std::shared_ptr<ASTNode>> statements;
};

class VariableAssignNode : public ASTNode {
    public:
        std::string name;
        std::shared_ptr<ASTNode> expression;
};
    
class IfNode : public ASTNode {
    public:
        std::shared_ptr<ASTNode> condition;
        std::shared_ptr<BlockNode> thenBlock;
        std::shared_ptr<BlockNode> elseBlock; // может быть nullptr
};
    
class ForNode : public ASTNode {
    public:
        std::string varName;
        std::shared_ptr<ASTNode> iterable;
        std::shared_ptr<BlockNode> body;
};
    
class ReturnNode : public ASTNode {
    public:
        std::shared_ptr<ASTNode> expression;
};
    
class CallNode : public ASTNode {
    public:
        std::string callee;
        std::vector<std::shared_ptr<ASTNode>> arguments;
};
    
class BinaryOpNode : public ASTNode {
    public:
        std::string op;
        std::shared_ptr<ASTNode> left;
        std::shared_ptr<ASTNode> right;
};
    
class UnaryOpNode : public ASTNode {
    public:
        std::string op;
        std::shared_ptr<ASTNode> operand;
};
    
class IdentifierNode : public ASTNode {
    public:
        std::string name;
};

class NumberNode : public ASTNode {
    public:
        int value;
};

class StringNode : public ASTNode {
    public:
        std::string value;
};

class BooleanNode : public ASTNode {
    public:
        bool value;
};
    
#endif // AST_H