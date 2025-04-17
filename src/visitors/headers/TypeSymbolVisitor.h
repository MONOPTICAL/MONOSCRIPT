#ifndef SYMANTICVISITOR_H
#define SYMANTICVISITOR_H

#include "../../parser/headers/AST.h"
#include "../../includes/icecream.hpp"
#include "Register.h"
#include "BuiltIn.h"

struct Context {
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> variables;
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> functions;
    std::string currentFunctionName;
    std::shared_ptr<TypeNode> returnType;
    bool returnedValue = false;
};

class TypeSymbolVisitor : public ASTNodeVisitor {

private:

    std::vector<Context>                                            contexts;

    std::vector<std::string>                                        types;

    Registry                                                        registry;

    /*
    Проверяет, существует ли переменная в реестре(если переданный node является идентификатором)
    */
    std::shared_ptr<TypeNode>                                       checkForIdentifier(std::shared_ptr<ASTNode>& node);

    void                                                            numCast(std::shared_ptr<ASTNode>& left 
                                                                    , std::shared_ptr<ASTNode>& right
                                                                    , const std::string& op);
    
    void                                                            validateCollectionElements(
                                                                    const std::shared_ptr<TypeNode>& expectedType,
                                                                    const std::shared_ptr<ASTNode>& expr,
                                                                        bool isAuto);

    void                                                            applyImplicitCastToNumeric(
                                                                        const std::shared_ptr<TypeNode>& expectedType,
                                                                        const std::shared_ptr<ASTNode>& expr,
                                                                        int maxRank,
                                                                        bool isAuto);
                                                                

public:
                                                                    TypeSymbolVisitor() 
                                                                    {
                                                                        // Инициализация контекста
                                                                        Context globalContext = {
                                                                            .variables = {},
                                                                            .functions = {},
                                                                            .currentFunctionName = "",
                                                                            .returnType = nullptr
                                                                        };
                                                                        contexts.push_back(globalContext);

                                                                        // Инициализация встроенных типов
                                                                        registerBuiltInTypes(registry);

                                                                        // Инициализация встроенных функций
                                                                        registerBuiltInFunctions(registry);
                                                                    };

    void                                                            debugContexts();
    
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
    void                                                            visit(NullNode& node) override;
    void                                                            visit(NoneNode& node) override;
    void                                                            visit(KeyValueNode& node) override;
    void                                                            visit(BreakNode& node) override;
    void                                                            visit(ContinueNode& node) override;
    void                                                            visit(AccessExpression& node) override;
};

#endif // SYMANTICVISITOR_H