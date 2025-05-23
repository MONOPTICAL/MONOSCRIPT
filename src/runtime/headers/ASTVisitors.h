#ifndef AST_VISITORS_H
#define AST_VISITORS_H

#include "../../parser/headers/AST.h"
#include "CodeGenContext.h"
#include <iostream>

// Visitor ответственный за генерацию кода
class ASTGen : public ASTNodeVisitor {
private:
    CodeGenContext&     context;
    llvm::Value*        result; // Текущий результат кодогенерации
    std::string         currentModuleName; // Имя текущей функции
    
public:
                        ASTGen(CodeGenContext& context);
    
    void                LogWarning(const std::string& message);

    // Получить результат последнего посещенного узла
    llvm::Value*        getResult() const;
    
    void                visit(SimpleTypeNode& node) override;
    void                visit(GenericTypeNode& node) override;
    void                visit(ProgramNode& node) override;
    void                visit(FunctionNode& node) override;
    void                visit(StructNode& node) override;
    void                visit(BlockNode& node) override;
    void                visit(VariableAssignNode& node) override;
    void                visit(ReassignMemberNode& node) override;
    void                visit(VariableReassignNode& node) override;
    void                visit(IfNode& node) override;
    void                visit(ForNode& node) override;
    void                visit(WhileNode& node) override;
    void                visit(ReturnNode& node) override;
    void                visit(CallNode& node) override;
    void                visit(BinaryOpNode& node) override;
    void                visit(UnaryOpNode& node) override;
    void                visit(IdentifierNode& node) override;
    void                visit(NumberNode& node) override;
    void                visit(FloatNumberNode& node) override;
    void                visit(StringNode& node) override;
    void                visit(NullNode& node) override;
    void                visit(NoneNode& node) override;
    void                visit(KeyValueNode& node) override;
    void                visit(BreakNode& node) override;
    void                visit(ContinueNode& node) override;
    void                visit(AccessExpression& node) override;
    void                visit(ImportNode& node) override;
    void                visit(LambdaNode& node) override;
    void                visit(ModuleMark& node) override;
};

#endif // AST_VISITORS_H