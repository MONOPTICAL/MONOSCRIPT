struct ASTNode {
    virtual ~ASTNode() = default;
};

struct CallNode : ASTNode {
    std::string func_name;
    std::vector<std::shared_ptr<ASTNode>> args;
};

struct VarAssignNode : ASTNode {
    std::string name;
    std::shared_ptr<ASTNode> expr;
};

struct IfNode : ASTNode {
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
};
