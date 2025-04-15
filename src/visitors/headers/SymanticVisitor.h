#ifndef SYMANTICVISITOR_H
#define SYMANTICVISITOR_H

#include "../../parser/headers/AST.h"
#include "../../includes/icecream.hpp"
#include "Register.h"

struct Context {
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> variables;
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> functions;
    std::string currentFunctionName;
    std::vector<std::string> functionArgs;
    std::string returnType;
};

class SymanticVisitor : public ASTNodeVisitor {

private:

    std::vector<Context>                                        contexts;

    std::vector<std::string>                                    types;

    Registry                                                    registry;
    
public:
                        SymanticVisitor() 
                        {
                            // Инициализация встроенных типов
                            registry.addBuiltinType("i8", std::make_shared<SimpleTypeNode>("i8"));
                            registry.addBuiltinType("i32", std::make_shared<SimpleTypeNode>("i32"));
                            registry.addBuiltinType("i64", std::make_shared<SimpleTypeNode>("i64"));
                            registry.addBuiltinType("float", std::make_shared<SimpleTypeNode>("float"));
                            registry.addBuiltinType("bool", std::make_shared<SimpleTypeNode>("bool"));
                            registry.addBuiltinType("string", std::make_shared<SimpleTypeNode>("string"));
                            registry.addBuiltinType("void", std::make_shared<SimpleTypeNode>("void"));
                            registry.addBuiltinType("null", std::make_shared<SimpleTypeNode>("null"));
                            registry.addBuiltinType("none", std::make_shared<SimpleTypeNode>("none"));

                        };
    
    void                                                            LogError(const std::string& message);

    void                                                            visit(SimpleTypeNode& node) override;
    void                                                            visit(GenericTypeNode& node) override;
    void                                                            visit(ProgramNode& node) override;
    void                                                            visit(FunctionNode& node) override;
    void                                                            visit(StructNode& node) override;
    void                                                            visit(BlockNode& node) override;
    void                                                            visit(VariableAssignNode& node) override;
    void                                                            visit(ReassignMemberNode& node) override;
    void                                                            visit(VariableReassignNode& node) override;
    void                                                            visit(IfNode& node) override;
    void                                                            visit(ForNode& node) override;
    void                                                            visit(WhileNode& node) override;
    void                                                            visit(ReturnNode& node) override;
    void                                                            visit(CallNode& node) override;
    void                                                            visit(BinaryOpNode& node) override;
    void                                                            visit(UnaryOpNode& node) override;
    void                                                            visit(IdentifierNode& node) override;
    void                                                            visit(NumberNode& node) override;
    void                                                            visit(FloatNumberNode& node) override;
    void                                                            visit(StringNode& node) override;
    void                                                            visit(BooleanNode& node) override;
    void                                                            visit(NullNode& node) override;
    void                                                            visit(NoneNode& node) override;
    void                                                            visit(KeyValueNode& node) override;
    void                                                            visit(BreakNode& node) override;
    void                                                            visit(ContinueNode& node) override;
    void                                                            visit(AccessExpression& node) override;
    void                                                            visit(ClassNode& node) override;
};




#endif // SYMANTICVISITOR_H