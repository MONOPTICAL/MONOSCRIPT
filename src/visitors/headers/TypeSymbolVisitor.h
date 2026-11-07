#ifndef SYMANTICVISITOR_H
#define SYMANTICVISITOR_H

#include <set>
#include <map>
#include "../../parser/headers/AST.h"
#include "../../errors/headers/ErrorEngine.h"
#include "../../loader/headers/loader.h"
#include "../../includes/ic.hpp"
#include "Register.h"
#include "BuiltIn.h"
#include <fstream>

struct Context {
    std::vector<std::string> labels;
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> variables;
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> functions;
    std::string currentFunctionName;
    std::shared_ptr<TypeNode> returnType;
    int resultHandlerDepth = 0;
    bool returnedValue = false;
};

inline bool isFloatType(const std::string& type) {
    return type == "float" || type == "f64";
}

class TypeSymbolVisitor : public ASTNodeVisitor {

private:
    std::unique_ptr<ErrorEngine>                                    errorEngine;

    std::vector<Context>                                            contexts;
    bool                                                            lastAccessResultField = false; // r.value и прочие поля result только для чтения
    bool                                                            checkResultLiteral(const std::shared_ptr<ASTNode>& node, const std::string& targetType);
    // Копии шаблонов: добавляются в программу после обхода, из какого модуля шаблон
    std::vector<std::shared_ptr<ASTNode>>                           genericInstances;
    std::map<std::string, std::string>                              genericModules;
    std::string                                                     instantiateGeneric(const std::shared_ptr<FunctionNode>& generic, CallNode& call,
                                                                        const std::vector<std::shared_ptr<TypeNode>>& argTypes);
    bool                                                            lastAccessIndexedString = false; // Последнее звено цепочки это s[i]: писать туда нельзя
    void                                                            checkCharCast(ASTNode& node);
    bool                                                            insideMethod = false;
    void                                                            checkCallArguments(std::vector<std::shared_ptr<ASTNode>>& arguments,
                                                                        const std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>>& parameters,
                                                                        size_t firstParameter, const std::string& name);
    // final без значения: присвоена точно, присвоена хотя бы на одном пути, и в каком цикле объявлена
    std::set<const ASTNode*>                                        assignedFinals;
    std::set<const ASTNode*>                                        maybeAssignedFinals;
    std::map<const ASTNode*, int>                                   finalLoopDepth;
    // Узлы живут до конца проверки: иначе адрес освобождённого занял бы новый узел и выглядел бы уже присвоенным
    std::vector<std::shared_ptr<ASTNode>>                           deferredFinals;
    int                                                             loopDepth = 0;
    void                                                            checkFinalAssigned(const std::shared_ptr<ASTNode>& variable, const std::string& name, const std::shared_ptr<ASTNode>& usage);

    std::vector<std::string>                                        types;

    Registry                                                        registry;

    std::shared_ptr<ProgramNode>                                    program; // Только для отладки

    std::string                                                     currentModuleName;

    // Все функции верхнего уровня по внутреннему имени, собранные до проверки тел
    std::unordered_map<std::string, std::shared_ptr<ASTNode>>       declaredFunctions;

    // Что видно каждому модулю: имя в коде -> функция. Свои функции плюс импортированные
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::shared_ptr<FunctionNode>>> moduleScopes;

    void                                                            collectDeclarations(ProgramNode& node);

    // В @pure нельзя завести новую ссылку на объект, который передали снаружи
    void                                                            checkPureAlias(const std::shared_ptr<ASTNode>& expr);

    /*
    Проверяет, существует ли переменная в реестре(если переданный node является идентификатором)
    */
   
    std::shared_ptr<TypeNode>                                       checkForIdentifier(std::shared_ptr<ASTNode>& node);

    void                                                            castNumbersInBinaryTree(
                                                                        std::shared_ptr<ASTNode>& node, 
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

    // Настоящий тип выражения; пустая строка - выражение только из литералов
    std::string                                                     resolveExprType(
                                                                            const std::shared_ptr<ASTNode>& node);

    // Единые правила приведения: ставит нужные касты или выдаёт ошибку
    void                                                            checkConversion(
                                                                            const std::shared_ptr<ASTNode>& node,
                                                                            const std::string& targetType,
                                                                            const std::string& errorPrefix = "");

    bool                                                            checkLabels(
                                                                            const std::string& label);

    void                                                            findMaxRank(
                                                                            const std::shared_ptr<ASTNode>& node, 
                                                                            int& maxRank);

    int                                                             getTypeRank(
                                                                        const std::string& type);

    std::string                                                     getTypeByRank(
                                                                        int rank);
    
    void                                                            castAllNumbersToType(
                                                                        const std::shared_ptr<ASTNode>& node, 
                                                                        const std::string& targetType); 

    std::shared_ptr<TypeNode>                                       get_common_type_from_list(
                                                                            std::vector<std::shared_ptr<TypeNode>>& types,
                                                                            std::shared_ptr<ASTNode> error_context_node, // Для LogError в правильном контексте
                                                                            bool allow_numeric_promotion_for_simple_types);
    std::shared_ptr<TypeNode>                                       infer_collection_type_revised(std::shared_ptr<BlockNode> block);

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

                                                                        // Поиск файла TOML
                                                                        std::string TOML_path = loader::findTomlPath();
                                                                        if (TOML_path.empty()) {
                                                                            std::cerr << "Warning: Не удалось найти путь к TOML-файлу" << std::endl;
                                                                            exit(1);
                                                                        }

                                                                        // Инициализация встроенных функций
                                                                        registerBuiltInFunctions(registry, TOML_path);
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
    void visit(AttemptNode& node) override;
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