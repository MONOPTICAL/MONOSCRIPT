#ifndef AST_H
#define AST_H
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <set>
#include <map>

class ASTNodeVisitor {
    public:
        virtual ~ASTNodeVisitor() = default;
        virtual void visit(class SimpleTypeNode& node) = 0;
        virtual void visit(class GenericTypeNode& node) = 0;
        virtual void visit(class ProgramNode& node) = 0;
        virtual void visit(class NamespaceNode& node) = 0;
        virtual void visit(class FunctionNode& node) = 0;
        virtual void visit(class StructNode& node) = 0;
        virtual void visit(class BlockNode& node) = 0;
        virtual void visit(class VariableAssignNode& node) = 0;
        virtual void visit(class ReassignMemberNode& node) = 0;
        virtual void visit(class VariableReassignNode& node) = 0;
        virtual void visit(class IfNode& node) = 0;
        virtual void visit(class AttemptNode& node) = 0;
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
class FunctionNode;

// u8..u64 хранятся без знака: расширяются нулями, делятся, сравниваются и сдвигаются вправо без знака
inline bool isUnsignedTypeName(const std::string& type) {
    return type == "u8" || type == "u16" || type == "u32" || type == "u64";
}
inline bool isIntegerTypeName(const std::string& type) {
    return type == "i1" || type == "i8" || type == "i16" || type == "i32" || type == "i64" || isUnsignedTypeName(type);
}
inline int integerWidth(const std::string& type) {
    if (type == "i1") return 1;
    if (type == "i8" || type == "u8") return 8;
    if (type == "i16" || type == "u16") return 16;
    if (type == "i32" || type == "u32") return 32;
    return 64;
}
// Неявно можно только без потерь: без знака в знаковое лишь более широкое, знаковое в беззнаковое никогда. i1 это 0 или 1
inline bool isLosslessInt(const std::string& from, const std::string& to) {
    if (from == to || from == "i1") return true;
    const bool fromUnsigned = isUnsignedTypeName(from), toUnsigned = isUnsignedTypeName(to);
    if (fromUnsigned == toUnsigned) return integerWidth(from) <= integerWidth(to);
    return fromUnsigned && integerWidth(from) < integerWidth(to);
}

class ASTNode : public std::enable_shared_from_this<ASTNode> {
    public:
        int line = 0; // номер строки в исходном коде
        int column = 0; // номер столбца в исходном коде

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

// Имена enum: в LLVM это i32, а семантика держит их отдельными типами. Заполняет парсер
inline std::set<std::string>& enumTypeNames() { static std::set<std::string> names; return names; }

class NamespaceNode: public ASTNode {
    public:
        std::string name;
        std::shared_ptr<ASTNode> body;
        bool isEnum = false; // [enum]Color: namespace из констант типа Color
        NamespaceNode() = default;
        NamespaceNode(const std::string& name, std::shared_ptr<ASTNode> body)
            : name(name), body(body) {}

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

        // Шаблон: [T]first<T>(array<T>: a). Сам он не проверяется и не генерируется.
        // instantiate заново разбирает функцию из токенов, подставляя вместо T конкретный тип:
        // копия получается как только что разобранный код, и семантика может свободно её менять
        std::vector<std::string> typeParameters;
        std::function<std::shared_ptr<FunctionNode>(const std::map<std::string, std::string>&)> instantiate;
        // Тело лямбды, вынесенное в функцию: захваченные значения приходят в объекте замыкания перед контекстом паники
        bool isLambda = false;
        std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> captures; // {тип, имя}
        
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
        std::shared_ptr<FunctionNode> lifted; // Функция верхнего уровня с телом лямбды, её создаёт семантика
    
        void accept(ASTNodeVisitor& visitor) override { visitor.visit(*this); }
};

class StructNode : public ASTNode {
    public:
        StructNode() = default;
        StructNode(const std::string& name, std::shared_ptr<ASTNode> body) : name(name), body(body) {}
        std::string name;
        std::shared_ptr<ASTNode> body;
        // Шаблон [struct]Stack<T>: копию Stack<i32> парсер собирает заново, подставив типы вместо T
        std::vector<std::string> typeParameters;
        std::function<std::shared_ptr<StructNode>(const std::map<std::string, std::string>&)> instantiate;

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
        bool isDeferred = false; // final без значения: присвоить можно ровно один раз, позже
        bool isCaptured = false; // Копия в лямбде: присвоить нельзя, а объект менять можно
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
    
class AttemptNode : public ASTNode {
public:
    std::shared_ptr<BlockNode> body;
    std::shared_ptr<BlockNode> errorBody;
    std::shared_ptr<BlockNode> panicBody;
    std::string errorName;
    std::string panicName;
    void accept(ASTNodeVisitor& visitor) override { visitor.visit(*this); }
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
        std::shared_ptr<TypeNode> functionValueType; // Вызов параметра типа func: keep(x)

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
        // Only postfix try: lazy replacement value or a terminating local handler.
        std::shared_ptr<ASTNode> fallback;
        std::shared_ptr<BlockNode> errorBody;

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};
    
class IdentifierNode : public ASTNode {
    public:
        IdentifierNode() = default;
        IdentifierNode(const std::string& name) : name(name) {}
        std::string name;
        std::shared_ptr<FunctionNode> referencedFunction; // Имя функции, переданной как значение: filter(a, isPositive)

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class NumberNode : public ASTNode {
    public:
        NumberNode() = default;
        NumberNode(int64_t value, std::shared_ptr<TypeNode> type) : value(value), type(type) {}
        int64_t value;
        std::shared_ptr<TypeNode> type; // тип числа (i32, i64, i8, i1)
        bool isBoolLiteral = false; // написано true или false, а не 0 или 1
        bool isCharLiteral = false; // написано 'a': это char, а не число, под другой тип не подстраивается

        void accept(ASTNodeVisitor& visitor) override {
            visitor.visit(*this);
        }
};

class FloatNumberNode : public ASTNode {
    public:
        FloatNumberNode() = default;
        FloatNumberNode(double value) : value(value) {}
        double value;

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
        bool isEmptyLiteral = false; // написано [] или {}, а не просто нет значения

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
        std::string baseName;
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