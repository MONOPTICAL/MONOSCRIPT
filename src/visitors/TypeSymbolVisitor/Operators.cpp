#include "../headers/TypeSymbolVisitor.h"

static std::shared_ptr<ASTNode> toStringHandler(std::shared_ptr<ASTNode>& node, TypeSymbolVisitor& visitor)
{
    std::shared_ptr<CallNode> callNode;
    std::shared_ptr<TypeNode> type;

    if(node->implicitCastTo)
        type = node->implicitCastTo;
    else
        type = node->inferredType;

    if (auto numberNode = std::dynamic_pointer_cast<NumberNode>(node)) {
        if(numberNode->isBoolLiteral)
            callNode = std::make_shared<CallNode>(
                "toString_bool",
                std::vector<std::shared_ptr<ASTNode>>{
                    std::make_shared<NumberNode>(numberNode->value, numberNode->inferredType)
                }
            );
        else
        {
            // Литерал печатаем как число: "n=" + 1 это n=1
            const bool isLong = type->toString() == "i64";
            auto literalType = std::make_shared<SimpleTypeNode>(isLong ? "i64" : "i32");
            auto literal = std::make_shared<NumberNode>(numberNode->value, literalType);
            literal->inferredType = literalType; // Этот узел семантика не обходит, а кодген читает его тип
            callNode = std::make_shared<CallNode>(
                isLong ? "toString_long" : "toString_int",
                std::vector<std::shared_ptr<ASTNode>>{literal}
            );
        }
    } else if (auto floatNumberNode = std::dynamic_pointer_cast<FloatNumberNode>(node)) {
        callNode = std::make_shared<CallNode>(
            "toString_float",
            std::vector<std::shared_ptr<ASTNode>>{
                std::make_shared<FloatNumberNode>(floatNumberNode->value)
            }
        );
    } else if (std::dynamic_pointer_cast<BinaryOpNode>(node) || std::dynamic_pointer_cast<UnaryOpNode>(node)) {
        auto binary = std::dynamic_pointer_cast<BinaryOpNode>(node);
        auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node);
        const bool isBoolean = (unary && unary->op != "neg") ||
            (binary && (binary->op.starts_with("icmp_") || binary->op.starts_with("fcmp_") ||
                        binary->op.starts_with("strcmp_") || binary->op == "and" || binary->op == "or"));

        std::string toStringMethod = "toString_int";
        const std::string typeName = type->toString();
        if (isBoolean) toStringMethod = "toString_bool";
        else if (typeName == "float") toStringMethod = "toString_float";
        else if (typeName == "i64") toStringMethod = "toString_long";
        else if (typeName != "i32") node->implicitCastTo = std::make_shared<SimpleTypeNode>("i32");

        callNode = std::make_shared<CallNode>(
            toStringMethod,
            std::vector<std::shared_ptr<ASTNode>>{node}
        );
    } else if (auto call = std::dynamic_pointer_cast<CallNode>(node)) {
        std::string toStringMethod;

        if (type->toString() == "float")
            toStringMethod = "toString_float";
        else if (type->toString() == "i1")
            toStringMethod = "toString_bool";
        else if (type->toString() == "i64")
            toStringMethod = "toString_long";
        else
            toStringMethod = "toString_int";

        callNode = std::make_shared<CallNode>(
            toStringMethod,
            std::vector<std::shared_ptr<ASTNode>>{
                std::make_shared<CallNode>(call->callee, call->arguments)
            }
        );
    } else if (std::dynamic_pointer_cast<IdentifierNode>(node) || std::dynamic_pointer_cast<AccessExpression>(node)) {
        std::string toStringMethod;
        const std::string typeName = type->toString();

        if (typeName == "float")
            toStringMethod = "toString_float";
        else if (typeName == "i1")
            toStringMethod = "toString_bool";
        else if (typeName == "i64")
            toStringMethod = "toString_long";
        else {
            toStringMethod = "toString_int";
            if (typeName != "i32") // i8 и i16 расширяем под параметр toString_int
                node->implicitCastTo = std::make_shared<SimpleTypeNode>("i32");
        }

        callNode = std::make_shared<CallNode>(
            toStringMethod,
            std::vector<std::shared_ptr<ASTNode>>{node}
        );
    }

    if (!callNode) 
        return node;

    return callNode;
}

// Над двумя i1 считаем как над числами: 1 + 1 это 2, а в самом i1 получился бы 0.
// Сравнения и логика остаются на i1
static std::shared_ptr<TypeNode> resultOfIntegerOperation(std::shared_ptr<BinaryOpNode>& node, const std::shared_ptr<TypeNode>& leftType, const std::shared_ptr<TypeNode>& rightType, Registry& registry) {
    if (leftType->toString() != "i1" || rightType->toString() != "i1") return leftType;

    auto leftLiteral = std::dynamic_pointer_cast<NumberNode>(node->left);
    auto rightLiteral = std::dynamic_pointer_cast<NumberNode>(node->right);

    // Только литералы: тип результата решит то, куда их присвоят, поэтому каста тут быть не должно. i8 b = 1 + 1 законно
    if (leftLiteral && rightLiteral) {
        auto i8Type = registry.findType("i8");
        leftLiteral->inferredType = i8Type;
        rightLiteral->inferredType = i8Type;
        return i8Type;
    }

    auto i32Type = registry.findType("i32");
    if (leftLiteral) leftLiteral->inferredType = i32Type;
    else node->left->implicitCastTo = i32Type;
    if (rightLiteral) rightLiteral->inferredType = i32Type;
    else node->right->implicitCastTo = i32Type;
    return i32Type;
}

void TypeSymbolVisitor::handlePlusOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    // string
    if (left == "string" || right == "string") {
        if (checkLabels("@strict") && !(left == "string" && right == "string"))
            LogError("Implicit type casting is not allowed for '+' in @strict mode: " + left + " and " + right, node->right->shared_from_this());

        // У объекта нет текста: без проверки ссылка ушла бы в scat как строка
        if ((left != "string" && getTypeRank(left) == 0) || (right != "string" && getTypeRank(right) == 0))
            LogError("Cannot join string with " + (left == "string" ? right : left), node->right->shared_from_this());
        
        // Операнды внутри выражения приводим друг к другу до превращения в строку
        if (left != "string") {
            if (std::dynamic_pointer_cast<BinaryOpNode>(node->left) || std::dynamic_pointer_cast<UnaryOpNode>(node->left))
                castNumbersInBinaryTree(node->left, "auto");
            node->left = toStringHandler(node->left, *this);
        }
        if (right != "string") {
            if (std::dynamic_pointer_cast<BinaryOpNode>(node->right) || std::dynamic_pointer_cast<UnaryOpNode>(node->right))
                castNumbersInBinaryTree(node->right, "auto");
            node->right = toStringHandler(node->right, *this);
        }
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "scat", node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("string");
        return;
    }

    // float
    if (left == "float" || right == "float") {
        if (checkLabels("@strict") && !(left == "float" && right == "float")) {
            bool leftImplicitFloat = node->left && node->left->implicitCastTo && node->left->implicitCastTo->toString() == "float";
            bool rightImplicitFloat = node->right && node->right->implicitCastTo && node->right->implicitCastTo->toString() == "float";
            if (leftImplicitFloat || rightImplicitFloat)
                LogError("Implicit te casting is not allowed for '+' in @strict mode: " + left + " and " + right, node->right->shared_from_this());
        }

        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "fadd", node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("float");
        return;
    }

    // int(i32, i64, i8, i1)
    if (isIntType(left) && isIntType(right)) {
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "add", node->right); node->implicitCastTo = cast; }
        node->inferredType = resultOfIntegerOperation(node, leftType, rightType, registry);
        return;
    }

    LogError("Unsupported operand types for '+': " + left + " and " + right, node->right->shared_from_this());
}

void TypeSymbolVisitor::handleMinusOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{

    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    // string
    if (left == "string" || right == "string") {
        LogError("Unsupported operand types for '-': " + left + " and " + right, node->right->shared_from_this());
        return;
    }

    // float
    if (left == "float" || right == "float") {
        if (checkLabels("@strict") && !(left == "float" && right == "float")) {
            bool leftImplicitFloat = node->left && node->left->implicitCastTo && node->left->implicitCastTo->toString() == "float";
            bool rightImplicitFloat = node->right && node->right->implicitCastTo && node->right->implicitCastTo->toString() == "float";
            if (leftImplicitFloat || rightImplicitFloat)
                LogError("Implicit type casting is not allowed for '-' in @strict mode: " + left + " and " + right, node->right->shared_from_this());
        }

        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "fsub", node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("float");
        return;
    }

    // int(i32, i64, i8, i1)
    if (isIntType(left) && isIntType(right)) {
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "sub", node->right); node->implicitCastTo = cast; }
        node->inferredType = resultOfIntegerOperation(node, leftType, rightType, registry);
        return;
    }

    LogError("Unsupported operand types for '+': " + left + " and " + right, node->right->shared_from_this());
}

void TypeSymbolVisitor::handleMulOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    // string
    if (left == "string" || right == "string") {
        LogError("Unsupported operand types for '*': " + left + " and " + right, node->right->shared_from_this());
        return;
    }

    // float
    if (left == "float" || right == "float") {
        if (checkLabels("@strict") && !(left == "float" && right == "float")) {
            bool leftImplicitFloat = node->left && node->left->implicitCastTo && node->left->implicitCastTo->toString() == "float";
            bool rightImplicitFloat = node->right && node->right->implicitCastTo && node->right->implicitCastTo->toString() == "float";
            if (leftImplicitFloat || rightImplicitFloat)
                LogError("Implicit type casting is not allowed for '*' in @strict mode: " + left + " and " + right, node->right->shared_from_this());
        }

        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "fmul", node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("float");
        return;
    }

    // int(i32, i64, i8, i1)
    if (isIntType(left) && isIntType(right)) {
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "mul", node->right); node->implicitCastTo = cast; }
        node->inferredType = resultOfIntegerOperation(node, leftType, rightType, registry);
        return;
    }

    LogError("Unsupported operand types for '+': " + left + " and " + right, node->right->shared_from_this());
}

void TypeSymbolVisitor::handleDivOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();
    
    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    if (left == "string" || right == "string") {
        LogError("Unsupported operand types for '/': " + left + " and " + right, node->right->shared_from_this());
        return;
    }

    if (left == "float" || right == "float") {
        if (checkLabels("@strict") && !(left == "float" && right == "float")) {
            bool leftImplicitFloat = node->left && node->left->implicitCastTo && node->left->implicitCastTo->toString() == "float";
            bool rightImplicitFloat = node->right && node->right->implicitCastTo && node->right->implicitCastTo->toString() == "float";
            if (leftImplicitFloat || rightImplicitFloat)
                LogError("Implicit type casting is not allowed for '/' in @strict mode: " + left + " and " + right, node->right->shared_from_this());
        }

        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "fdiv", node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("float");
        return;
    }

    if (isIntType(left) && isIntType(right)) {
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "sdiv", node->right); node->implicitCastTo = cast; }
        node->inferredType = resultOfIntegerOperation(node, leftType, rightType, registry);
        return;
    }

    LogError("Unsupported operand types for '/': " + left + " and " + right, node->right->shared_from_this());
}

void TypeSymbolVisitor::handleModOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    if (left == "string" || right == "string") {
        LogError("Unsupported operand types for '%': " + left + " and " + right, node->right->shared_from_this());
        return;
    }

    if (left == "float" || right == "float") {
        if (checkLabels("@strict") && !(left == "float" && right == "float")) {
            bool leftImplicitFloat = node->left && node->left->implicitCastTo && node->left->implicitCastTo->toString() == "float";
            bool rightImplicitFloat = node->right && node->right->implicitCastTo && node->right->implicitCastTo->toString() == "float";
            if (leftImplicitFloat || rightImplicitFloat)
                LogError("Implicit type casting is not allowed for '%' in @strict mode: " + left + " and " + right, node->right->shared_from_this());
        }

        if (left != "float") {
            node->left->implicitCastTo = registry.findType("float");
        }
        if (right != "float") {
            node->right->implicitCastTo = registry.findType("float");
        }
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "frem", node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("float");
        return;
    }

    if (isIntType(left) && isIntType(right)) {
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "srem", node->right); node->implicitCastTo = cast; }
        node->inferredType = resultOfIntegerOperation(node, leftType, rightType, registry);
        return;
    }

    LogError("Unsupported operand types for '%': " + left + " and " + right, node->right->shared_from_this());
}

void TypeSymbolVisitor::handleCompareOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    auto isIntType = [](const std::string& t) {
        return t == "i32" || t == "i64" || t == "i16" || t == "i8" || t == "i1";
    };

    // IR-операции по node->op
    std::string cmpOp;
    if (node->op == "==" || node->op == "eq") cmpOp = "eq";
    else if (node->op == "!=" || node->op == "ne") cmpOp = "ne";
    else if (node->op == "<") cmpOp = "slt";
    else if (node->op == ">") cmpOp = "sgt";
    else if (node->op == "<=") cmpOp = "sle";
    else if (node->op == ">") cmpOp = "sgt";
    else if (node->op == ">=") cmpOp = "sge";
    else cmpOp = ""; // неизвестный оператор

    if (cmpOp.empty()) {
        LogError("Unknown comparison operator: " + node->op, node->right->shared_from_this());
        return;
    }

    // string
    if (left == "string" || right == "string") {
        if ((left != "string" && getTypeRank(left) == 0) || (right != "string" && getTypeRank(right) == 0))
            LogError("Unsupported operand types for comparison: " + left + " and " + right, node->right->shared_from_this());

        // Число сравнивается со строкой как текст, тем же способом, что и в склейке
        if (left != "string") {
            node->left = toStringHandler(node->left, *this);
            node->left->accept(*this);
        }
        if (right != "string") {
            node->right = toStringHandler(node->right, *this);
            node->right->accept(*this);
        }
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "strcmp_" + cmpOp, node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("i1");
        return;
    }

    // float
    if (left == "float" || right == "float") {
        if (checkLabels("@strict") && !(left == "float" && right == "float")) {
            bool leftImplicitFloat = node->left && node->left->implicitCastTo && node->left->implicitCastTo->toString() == "float";
            bool rightImplicitFloat = node->right && node->right->implicitCastTo && node->right->implicitCastTo->toString() == "float";
            if (leftImplicitFloat || rightImplicitFloat)
                LogError("Implicit type casting is not allowed for '" + node->op + "' in @strict mode: " + left + " and " + right, node->right->shared_from_this());
        }

        if (left != "float") node->left->implicitCastTo = registry.findType("float");
        if (right != "float") node->right->implicitCastTo = registry.findType("float");
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "fcmp_" + cmpOp, node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("i1");
        return;
    }

    // int(i32, i64, i8, i1)
    if (isIntType(left) && isIntType(right)) {
        { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, "icmp_" + cmpOp, node->right); node->implicitCastTo = cast; }
        node->inferredType = registry.findType("i1");
        return;
    }

    LogError("Unsupported operand types for comparison: " + left + " and " + right, node->right->shared_from_this());
}

void TypeSymbolVisitor::handleLogicalOperator(std::shared_ptr<BinaryOpNode>& node, std::shared_ptr<TypeNode> leftType, std::shared_ptr<TypeNode> rightType)
{
    std::string left = leftType->toString();
    std::string right = rightType->toString();

    if (left != "i1" || right != "i1") {
        LogError("Logical operators require boolean operands, got: " + left + " and " + right, node->right->shared_from_this());
        return;
    }

    // and, or
    std::string op = node->op;
    if (op == "and") op = "and";
    else if (op == "or") op = "or";
    else {
        LogError("Unknown logical operator: " + node->op, node->right->shared_from_this());
        return;
    }

    { auto cast = node->implicitCastTo; node = std::make_shared<BinaryOpNode>(node->left, op, node->right); node->implicitCastTo = cast; }
    node->inferredType = registry.findType("i1");
    node->implicitCastTo = registry.findType("i1");
}