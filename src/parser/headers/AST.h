#ifndef AST_H
#define AST_H
#include <memory>
#include <string>
#include <vector>
#include <map>

class ASTNodeVisitor {
    public:
        virtual ~ASTNodeVisitor() = default;
        virtual void visit(class SimpleTypeNode& node) = 0;
        virtual void visit(class GenericTypeNode& node) = 0;
        virtual void visit(class ProgramNode& node) = 0;
        virtual void visit(class FunctionNode& node) = 0;
        virtual void visit(class StructNode& node) = 0;
        virtual void visit(class BlockNode& node) = 0;
        virtual void visit(class VariableAssignNode& node) = 0;
        virtual void visit(class ReassignMemberNode& node) = 0;
        virtual void visit(class VariableReassignNode& node) = 0;
        virtual void visit(class IfNode& node) = 0;
        virtual void visit(class ForNode& node) = 0;
        virtual void visit(class WhileNode& node) = 0;
        virtual void visit(class ReturnNode& node) = 0;
        virtual void visit(class CallNode& node) = 0;
        virtual void visit(class BinaryOpNode& node) = 0;
        virtual void visit(class UnaryOpNode& node) = 0;
        virtual void visit(class IdentifierNode& node) = 0;
        virtual void visit(class NumberNode& node) = 0;
        virtual void visit(class FloatNumberNode& node) = 0;
        virtual void visit(class StringNode& node) = 0;
        virtual void visit(class NullNode& node) = 0;
        virtual void visit(class NoneNode& node) = 0;
        virtual void visit(class KeyValueNode& node) = 0;
        virtual void visit(class BreakNode& node) = 0;
        virtual void visit(class ContinueNode& node) = 0;
        virtual void visit(class AccessExpression& node) = 0;
        virtual void visit(class ImportNode& node) = 0;
        virtual void visit(class LambdaNode& node) = 0;
        virtual void visit(class ModuleMark& node) = 0;
};

class TypeNode;

class ASTNode : public std::enable_shared_from_this<ASTNode> {
    public:
        int line; // номер строки в исходном коде
        int column; // номер столбца в исходном коде

        std::shared_ptr<TypeNode> inferredType; // Выводимый тип узла (для IR)

        std::shared_ptr<TypeNode> implicitCastTo; // Неявное приведение к типу (для IR)

        

        virtual ~ASTNode() = default;

        virtual void accept(ASTNodeVisitor& visitor) = 0; // Метод для обхода узла

        std::shared_ptr<ASTNode> shared_from_this() {
            return std::enable_shared_from_this<ASTNode>::shared_from_this();
        }
};

class ModuleMark : public ASTNode {
    public:
        ModuleMark() = default;
        ModuleMark(const std::string& moduleName) : moduleName(moduleName) {}
        std::string moduleName;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class TypeNode : public ASTNode {
    public:
        TypeNode() = default;
        virtual ~TypeNode() = default;
        virtual std::string toString() const = 0; // Printing type
};

class SimpleTypeNode : public TypeNode {
    public:
        SimpleTypeNode(const std::string& name) : name(name) {}
        std::string name;

        std::string toString() const override {
            return name;
        }

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class GenericTypeNode : public TypeNode {
    public:
        GenericTypeNode(const std::string& baseName) : baseName(baseName) {}

        std::string baseName;
        std::vector<std::shared_ptr<TypeNode>> typeParameters;

        std::string toString() const override {
            std::string result = baseName + "<";
            for(size_t i=0; i < typeParameters.size(); ++i)
            {
                    if(i>0) result += ", ";
                    result += typeParameters[i]->toString();
            }
            result += ">";
            return result;
        }

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class ProgramNode : public ASTNode {
    public:
        ProgramNode() = default;
        ProgramNode(
            const std::vector<std::shared_ptr<ASTNode>>& body,
            std::string moduleName) 
            : body(body), moduleName(moduleName) {}
        std::vector<std::shared_ptr<ASTNode>> body;
        std::string moduleName;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class FunctionNode : public ASTNode {
    public:
        FunctionNode() = default;
        FunctionNode(const std::string& name, const std::string& associated, std::shared_ptr<TypeNode> returnType, const std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters, const std::vector<std::string> labels, std::shared_ptr<ASTNode> body) 
            : name(name), associated(associated), returnType(returnType), parameters(parameters), labels(labels), body(body) {}
        std::string name;
        std::string associated;
        std::shared_ptr<TypeNode> returnType;
        std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters; // {type, name}
        std::vector<std::pair<std::string, LambdaNode>> lambdas; // {type, name}

        std::vector<std::string> labels; // @strict, @pure, @entry, @public, @private, @test
        std::shared_ptr<ASTNode> body; // BlockNode
        
        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class LambdaNode : public ASTNode {
    public:
        std::shared_ptr<TypeNode> returnType;
        std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters; // {type, name}
        std::shared_ptr<ASTNode> body;
    
        LambdaNode(std::shared_ptr<TypeNode> returnType,
                            const std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters,
                            std::shared_ptr<ASTNode> body)
            : returnType(returnType), parameters(parameters), body(body) {}
    
        void accept(ASTNodeVisitor& visitor) override { visitor.visit(*this); }
};

class StructNode : public ASTNode {
    public:
        StructNode() = default;
        StructNode(const std::string& name, std::shared_ptr<ASTNode> body) : name(name), body(body) {}
        std::string name;
        std::shared_ptr<ASTNode> body;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class BlockNode : public ASTNode {
    public:
        BlockNode() = default;
        BlockNode(const std::vector<std::shared_ptr<ASTNode>>& statements) : statements(statements) {}
        std::vector<std::shared_ptr<ASTNode>> statements;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class VariableAssignNode : public ASTNode {
    public:
        VariableAssignNode() = default;
        VariableAssignNode(const std::string& name, bool isConst, std::shared_ptr<TypeNode> type, std::shared_ptr<ASTNode> expression) 
            : name(name), isConst(isConst), type(type), expression(expression) {}
        std::string name;
        std::shared_ptr<TypeNode> type;
        bool isConst;
        std::shared_ptr<ASTNode> expression;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class ReassignMemberNode : public ASTNode {
    public:
        ReassignMemberNode() = default;
        ReassignMemberNode(std::shared_ptr<ASTNode> accessExpression, std::shared_ptr<ASTNode> expression) 
            : accessExpression(accessExpression), expression(expression) {}
        std::shared_ptr<ASTNode> accessExpression;
        std::shared_ptr<ASTNode> expression;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class VariableReassignNode : public ASTNode {
    public:
        VariableReassignNode() = default;
        VariableReassignNode(const std::string& name, std::shared_ptr<ASTNode> expression) 
            : name(name), expression(expression) {}
        std::string name;
        std::shared_ptr<ASTNode> expression;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class IfNode : public ASTNode {
    public:
        IfNode() = default;
        IfNode(std::shared_ptr<ASTNode> condition, std::shared_ptr<BlockNode> thenBlock, std::shared_ptr<ASTNode> elseBlock = nullptr) 
            : condition(condition), thenBlock(thenBlock), elseBlock(elseBlock) {}
        std::shared_ptr<ASTNode> condition;
        std::shared_ptr<BlockNode> thenBlock;
        std::shared_ptr<ASTNode> elseBlock; // может быть и if

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class ForNode : public ASTNode {
    public:
        ForNode() = default;
        ForNode(const std::string& varName, std::shared_ptr<ASTNode> iterable, std::shared_ptr<BlockNode> body) 
            : varName(varName), iterable(iterable), body(body) {}
        std::string varName;
        std::shared_ptr<TypeNode> varType; // TODO: <-- чек хуету
        std::shared_ptr<ASTNode> iterable;
        std::shared_ptr<BlockNode> body;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class WhileNode : public ASTNode {
    public:
        WhileNode() = default;
        WhileNode(std::shared_ptr<ASTNode> condition, std::shared_ptr<BlockNode> body)
            : condition(condition), body(body) {}
        std::shared_ptr<ASTNode> condition;
        std::shared_ptr<BlockNode> body;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class ReturnNode : public ASTNode {
    public:
        ReturnNode() = default;
        ReturnNode(std::shared_ptr<ASTNode> expression) : expression(expression) {}
        std::shared_ptr<ASTNode> expression;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class CallNode : public ASTNode {
    public:
        CallNode() = default;
        CallNode(const std::string& callee, const std::vector<std::shared_ptr<ASTNode>>& arguments) 
            : callee(callee), arguments(arguments) {}
        std::string callee;
        std::vector<std::shared_ptr<ASTNode>> arguments;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class BinaryOpNode : public ASTNode {
    public:
        BinaryOpNode() = default;
        BinaryOpNode(std::shared_ptr<ASTNode> left, const std::string& op, std::shared_ptr<ASTNode> right) 
            : left(left), op(op), right(right) {}
        std::string op;
        std::shared_ptr<ASTNode> left;
        std::shared_ptr<ASTNode> right;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class UnaryOpNode : public ASTNode {
    public:
        UnaryOpNode() = default;
        UnaryOpNode(const std::string& op, std::shared_ptr<ASTNode> operand) 
            : op(op), operand(operand) {}
        std::string op;
        std::shared_ptr<ASTNode> operand;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class IdentifierNode : public ASTNode {
    public:
        IdentifierNode() = default;
        IdentifierNode(const std::string& name) : name(name) {}
        std::string name;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class NumberNode : public ASTNode {
    public:
        NumberNode() = default;
        NumberNode(int value, std::shared_ptr<TypeNode> type) : value(value), type(type) {}
        int value;
        std::shared_ptr<TypeNode> type; // тип числа (i32, i64, i8, i1)


        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class FloatNumberNode : public ASTNode {
    public:
        FloatNumberNode() = default;
        FloatNumberNode(float value) : value(value) {}
        float value;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class StringNode : public ASTNode {
    public:
        StringNode() = default;
        StringNode(const std::string& value) : value(value) {}
        std::string value;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class NullNode : public ASTNode {
    public:
        NullNode() = default;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class NoneNode : public ASTNode {
    public:
        NoneNode() = default;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class KeyValueNode : public ASTNode {
    public:
        KeyValueNode() = default;

        std::shared_ptr<ASTNode> key;
        std::shared_ptr<ASTNode> value;

        std::string keyName;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class BreakNode : public ASTNode {
    public:
        BreakNode() = default;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class ContinueNode : public ASTNode {
    public:
        ContinueNode() = default;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
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
        std::shared_ptr<ASTNode> nextAccess;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class ImportNode : public ASTNode {
    public:
        ImportNode() = default;
        ImportNode(const std::map<std::vector<std::string>, std::string>& paths)
        : paths(paths){}
        
        // Путь импорта: module -> struct -> function ...
        std::map<std::vector<std::string>, std::string> paths;
    

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

#endif // AST_H