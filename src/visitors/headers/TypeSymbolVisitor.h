#ifndef SYMANTICVISITOR_H
#define SYMANTICVISITOR_H

#include "../../parser/headers/AST.h"
#include "../../errors/headers/ErrorEngine.h"
#include "../../includes/icecream.hpp"
#include "Register.h"
#include "BuiltIn.h"

struct Context {
    std::vector<std::string> labels;
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> variables;
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> functions;
    std::string currentFunctionName;
    std::shared_ptr<TypeNode> returnType;
    bool returnedValue = false;
};

class TypeSymbolVisitor : public ASTNodeVisitor {

private:
    std::unique_ptr<ErrorEngine>                                    errorEngine;

    std::vector<Context>                                            contexts;

    std::vector<std::string>                                        types;

    Registry                                                        registry;

    std::shared_ptr<ProgramNode>                                    program; // Только для отладки

    std::string                                                     currentModuleName;

    /*
    Проверяет, существует ли переменная в реестре(если переданный node является идентификатором)
    */
   
    std::shared_ptr<TypeNode>                                       checkForIdentifier(std::shared_ptr<ASTNode>& node);

    void                                                            castNumbersInBinaryTree(
                                                                        std::shared_ptr<ASTNode> node, 
                                                                        const std::string& expectedType);
    
    void                                                            validateCollectionElements(
                                                                        const std::shared_ptr<TypeNode>& expectedType,
                                                                        const std::shared_ptr<ASTNode>& expr,
                                                                        bool isAuto);

    void                                                            applyImplicitCastToNumeric(
                                                                        const std::shared_ptr<TypeNode>& expectedType,
                                                                        const std::shared_ptr<ASTNode>& expr,
                                                                        int maxRank,
                                                                        bool isAuto);

    void                                                            handlePlusOperator(
                                                                        std::shared_ptr<BinaryOpNode>& node, 
                                                                        std::shared_ptr<TypeNode> leftType, 
                                                                        std::shared_ptr<TypeNode> rightType);

    void                                                            handleMinusOperator(
                                                                            std::shared_ptr<BinaryOpNode>& node, 
                                                                            std::shared_ptr<TypeNode> leftType, 
                                                                            std::shared_ptr<TypeNode> rightType);

    void                                                            handleMulOperator(
                                                                            std::shared_ptr<BinaryOpNode>& node, 
                                                                            std::shared_ptr<TypeNode> leftType, 
                                                                            std::shared_ptr<TypeNode> rightType);
                                                                            
    void                                                            handleDivOperator(
                                                                            std::shared_ptr<BinaryOpNode>& node, 
                                                                            std::shared_ptr<TypeNode> leftType, 
                                                                            std::shared_ptr<TypeNode> rightType);

    void                                                            handleModOperator(
                                                                            std::shared_ptr<BinaryOpNode>& node, 
                                                                            std::shared_ptr<TypeNode> leftType, 
                                                                            std::shared_ptr<TypeNode> rightType);
    
    void                                                            handleCompareOperator(
                                                                            std::shared_ptr<BinaryOpNode>& node, 
                                                                            std::shared_ptr<TypeNode> leftType, 
                                                                            std::shared_ptr<TypeNode> rightType);

    void                                                            handleLogicalOperator(
                                                                            std::shared_ptr<BinaryOpNode>& node, 
                                                                            std::shared_ptr<TypeNode> leftType, 
                                                                            std::shared_ptr<TypeNode> rightType);

    void                                                            castAndValidate(
                                                                            const std::shared_ptr<ASTNode>& node, 
                                                                            const std::string& targetType, 
                                                                            TypeSymbolVisitor* visitor);

    bool                                                            checkLabels(
                                                                            const std::string& label);

    void                                                            findMaxRank(
                                                                            const std::shared_ptr<ASTNode>& node, 
                                                                            int& maxRank);
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
                                                                        registerBuiltInFunctions(registry, STDLIB_TOML_PATH);
                                                                    };

    void                                                            debugContexts();
    
    void                                                            LogError(const std::string& message, std::shared_ptr<ASTNode> node = nullptr);

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
    void                                                            visit(ImportNode& node) override;
    void                                                            visit(LambdaNode& node) override;
    void                                                            visit(ModuleMark& node) override;
};

#endif // SYMANTICVISITOR_H