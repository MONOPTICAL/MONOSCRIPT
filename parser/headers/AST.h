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
        ProgramNode() = default;
        ProgramNode(const std::vector<std::shared_ptr<ASTNode>>& body) : body(body) {}
        std::vector<std::shared_ptr<ASTNode>> body;
};
    
class FunctionNode : public ASTNode {
    public:
        FunctionNode() = default;
        FunctionNode(const std::string& name, const std::string& returnType, const std::vector<std::pair<std::string, std::string>>& parameters, std::shared_ptr<ASTNode> body) 
            : name(name), returnType(returnType), parameters(parameters), body(body) {}
        std::string name;
        std::string returnType;
        std::vector<std::pair<std::string, std::string>> parameters; // {type, name}
        std::shared_ptr<ASTNode> body; // BlockNode
};

class StructNode : public ASTNode {
    public:
        StructNode() = default;
        StructNode(const std::string& name, std::shared_ptr<ASTNode> body) : name(name), body(body) {}
        std::string name;
        std::shared_ptr<ASTNode> body;
};

class BlockNode : public ASTNode {
    public:
        BlockNode() = default;
        BlockNode(const std::vector<std::shared_ptr<ASTNode>>& statements) : statements(statements) {}
        std::vector<std::shared_ptr<ASTNode>> statements;
};

class VariableAssignNode : public ASTNode {
    public:
        VariableAssignNode() = default;
        VariableAssignNode(const std::string& name, bool isConst, const std::string& type, std::shared_ptr<ASTNode> expression) 
            : name(name), isConst(isConst), type(type), expression(expression) {}
        std::string name;
        std::string type;
        bool isConst;
        std::shared_ptr<ASTNode> expression;
};

class VariableReassignNode : public ASTNode {
    public:
        VariableReassignNode() = default;
        VariableReassignNode(const std::string& name, std::shared_ptr<ASTNode> expression) 
            : name(name), expression(expression) {}
        std::string name;
        std::shared_ptr<ASTNode> expression;
};
    
class IfNode : public ASTNode {
    public:
        IfNode() = default;
        IfNode(std::shared_ptr<ASTNode> condition, std::shared_ptr<BlockNode> thenBlock, std::shared_ptr<BlockNode> elseBlock = nullptr) 
            : condition(condition), thenBlock(thenBlock), elseBlock(elseBlock) {}
        std::shared_ptr<ASTNode> condition;
        std::shared_ptr<BlockNode> thenBlock;
        std::shared_ptr<BlockNode> elseBlock; // может быть nullptr
};
    
class ForNode : public ASTNode {
    public:
        ForNode() = default;
        ForNode(const std::string& varName, std::shared_ptr<ASTNode> iterable, std::shared_ptr<BlockNode> body) 
            : varName(varName), iterable(iterable), body(body) {}
        std::string varName;
        std::shared_ptr<ASTNode> iterable;
        std::shared_ptr<BlockNode> body;
};

class WhileNode : public ASTNode {
    public:
        WhileNode() = default;
        WhileNode(std::shared_ptr<ASTNode> condition, std::shared_ptr<BlockNode> body)
            : condition(condition), body(body) {}
        std::shared_ptr<ASTNode> condition;
        std::shared_ptr<BlockNode> body;

};
    
class ReturnNode : public ASTNode {
    public:
        ReturnNode() = default;
        ReturnNode(std::shared_ptr<ASTNode> expression) : expression(expression) {}
        std::shared_ptr<ASTNode> expression;
};
    
class CallNode : public ASTNode {
    public:
        CallNode() = default;
        CallNode(const std::string& callee, const std::vector<std::shared_ptr<ASTNode>>& arguments) 
            : callee(callee), arguments(arguments) {}
        std::string callee;
        std::vector<std::shared_ptr<ASTNode>> arguments;
};
    
class BinaryOpNode : public ASTNode {
    public:
        BinaryOpNode() = default;
        BinaryOpNode(std::shared_ptr<ASTNode> left, const std::string& op, std::shared_ptr<ASTNode> right) 
            : left(left), op(op), right(right) {}
        std::string op;
        std::shared_ptr<ASTNode> left;
        std::shared_ptr<ASTNode> right;
};
    
class UnaryOpNode : public ASTNode {
    public:
        UnaryOpNode() = default;
        UnaryOpNode(const std::string& op, std::shared_ptr<ASTNode> operand) 
            : op(op), operand(operand) {}
        std::string op;
        std::shared_ptr<ASTNode> operand;
};
    
class IdentifierNode : public ASTNode {
    public:
        IdentifierNode() = default;
        IdentifierNode(const std::string& name) : name(name) {}
        std::string name;
};

class NumberNode : public ASTNode {
    public:
        NumberNode() = default;
        NumberNode(int value) : value(value) {}
        int value;
};

class FloatNumberNode : public ASTNode {
    public:
        FloatNumberNode() = default;
        FloatNumberNode(float value) : value(value) {}
        float value;
};

class StringNode : public ASTNode {
    public:
        StringNode() = default;
        StringNode(const std::string& value) : value(value) {}
        std::string value;
};

class BooleanNode : public ASTNode {
    public:
        BooleanNode() = default;
        BooleanNode(bool value) : value(value) {}
        bool value;
};

class NullNode : public ASTNode {
    public:
        NullNode() = default;
};

class NoneNode : public ASTNode {
    public:
        NoneNode() = default;
};

class KeyValueNode : public ASTNode {
    public:
        KeyValueNode() = default;
        std::shared_ptr<ASTNode> key;
        std::shared_ptr<ASTNode> value;
};

class BreakNode : public ASTNode {
    public:
        BreakNode() = default;
};
    
class ContinueNode : public ASTNode {
    public:
        ContinueNode() = default;
};

// Ну тут короче и dot и [] должны быть
// Я рот ебал их разделять
class AccessExpression : public ASTNode {
    public: 
        AccessExpression() = default;
        AccessExpression(std::string memberName, std::shared_ptr<ASTNode> expression)
            :   memberName(memberName), expression(expression) {};
        std::string memberName;
        std::string notation;
        std::shared_ptr<ASTNode> expression;
};

#endif // AST_H