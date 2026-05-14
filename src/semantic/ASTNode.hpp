#pragma once

#include "../common/DataType.hpp"
#include <memory>
#include <string>
#include <vector>

// AST Node Types
enum class ASTNodeType {
    Program,
    Block,
    VarDecl,
    ProcDecl,
    FuncDecl,
    CompoundStmt,
    AssignmentStmt,
    IfStmt,
    WhileStmt,
    RepeatStmt,
    ForStmt,
    CallStmt,
    Literal,
    Variable,
    BinaryOp,
    UnaryOp,
    FuncCall,
    Error
};


// Base AST Node
class ASTNode {
public:
    ASTNodeType nodeType;
    int scopeLevel = -1;
    std::vector<std::string> semanticErrors;

    ASTNode(ASTNodeType t) : nodeType(t) {}
    virtual ~ASTNode() = default;
};

class ExpressionNode : public ASTNode {
public:
    DataType dataType = DataType::UNKNOWN;

    ExpressionNode(ASTNodeType t)
        : ASTNode(t) {}
};

class LiteralNode : public ExpressionNode {
public:
    std::string value;

    LiteralNode(DataType dt, const std::string& val)
        : ExpressionNode(ASTNodeType::Literal),
          value(val)
    {
        dataType = dt;
    }
};

class VariableNode : public ExpressionNode {
public:
    std::string name;
    int tabIndex = -1;

    VariableNode(const std::string& n)
        : ExpressionNode(ASTNodeType::Variable),
          name(n) {}
};

class BinaryOpNode : public ExpressionNode {
public:
    std::string op;

    std::shared_ptr<ExpressionNode> left;
    std::shared_ptr<ExpressionNode> right;

    BinaryOpNode(
        const std::string& o,
        std::shared_ptr<ExpressionNode> l,
        std::shared_ptr<ExpressionNode> r
    )
        : ExpressionNode(ASTNodeType::BinaryOp),
          op(o),
          left(l),
          right(r) {}
};

class UnaryOpNode : public ExpressionNode {
public:
    std::string op;
    std::shared_ptr<ExpressionNode> operand;

    UnaryOpNode(
        const std::string& o,
        std::shared_ptr<ExpressionNode> expr
    )
        : ExpressionNode(ASTNodeType::UnaryOp),
          op(o),
          operand(expr) {}
};

class FunctionCallNode : public ExpressionNode {
public:
    std::string name;
    int tabIndex = -1;

    std::vector<std::shared_ptr<ExpressionNode>> args;

    FunctionCallNode(const std::string& n)
        : ExpressionNode(ASTNodeType::FuncCall),
          name(n) {}
};

// Statement
class StatementNode : public ASTNode {
public:
    StatementNode(ASTNodeType t)
        : ASTNode(t) {}
};

class AssignmentNode : public StatementNode {
public:
    std::shared_ptr<VariableNode> target;
    std::shared_ptr<ExpressionNode> value;

    AssignmentNode(
        std::shared_ptr<VariableNode> t,
        std::shared_ptr<ExpressionNode> v
    )
        : StatementNode(ASTNodeType::AssignmentStmt),
          target(t),
          value(v) {}
};

class CompoundStatementNode : public StatementNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements;

    CompoundStatementNode()
        : StatementNode(ASTNodeType::CompoundStmt) {}
};

class IfStatementNode : public StatementNode {
public:
    std::shared_ptr<ExpressionNode> condition;
    std::shared_ptr<StatementNode> thenPart;
    std::shared_ptr<StatementNode> elsePart;

    IfStatementNode(
        std::shared_ptr<ExpressionNode> cond,
        std::shared_ptr<StatementNode> thenStmt,
        std::shared_ptr<StatementNode> elseStmt = nullptr
    )
        : StatementNode(ASTNodeType::IfStmt),
          condition(cond),
          thenPart(thenStmt),
          elsePart(elseStmt) {}
};

class WhileStatementNode : public StatementNode {
public:
    std::shared_ptr<ExpressionNode> condition;
    std::shared_ptr<StatementNode> body;

    WhileStatementNode(
        std::shared_ptr<ExpressionNode> cond,
        std::shared_ptr<StatementNode> b
    )
        : StatementNode(ASTNodeType::WhileStmt),
          condition(cond),
          body(b) {}
};

class RepeatStatementNode : public StatementNode {
public:
    std::shared_ptr<CompoundStatementNode> body;
    std::shared_ptr<ExpressionNode> condition;

    RepeatStatementNode(
        std::shared_ptr<CompoundStatementNode> b,
        std::shared_ptr<ExpressionNode> c
    )
        : StatementNode(ASTNodeType::RepeatStmt),
          body(b),
          condition(c) {}
};

class ForStatementNode : public StatementNode {
public:
    std::shared_ptr<VariableNode> controlVar;
    std::shared_ptr<ExpressionNode> startExpr;
    std::shared_ptr<ExpressionNode> endExpr;
    std::shared_ptr<StatementNode> body;

    bool isDownto;

    ForStatementNode(...)
        : StatementNode(ASTNodeType::ForStmt) {}
};

class ProcedureCallNode : public StatementNode {
public:
    std::string name;
    int tabIndex = -1;

    std::vector<std::shared_ptr<ExpressionNode>> args;

    ProcedureCallNode(const std::string& n)
        : StatementNode(ASTNodeType::CallStmt),
          name(n) {}
};

// Declarations
class DeclarationNode : public ASTNode {
public:
    DeclarationNode(ASTNodeType t)
        : ASTNode(t) {}
};

class VarDeclarationNode : public DeclarationNode {
public:
    std::vector<std::string> names;
    DataType declaredType;

    int atabRef = -1;

    VarDeclarationNode(
        const std::vector<std::string>& ids,
        DataType dt
    )
        : DeclarationNode(ASTNodeType::VarDecl),
          names(ids),
          declaredType(dt) {}
};

class ProcedureDeclarationNode : public DeclarationNode {
public:
    std::string name;
    int btabRef = -1;

    std::shared_ptr<BlockNode> body;

    ProcedureDeclarationNode(const std::string& n)
        : DeclarationNode(ASTNodeType::ProcDecl),
          name(n) {}
};

class FunctionDeclarationNode : public DeclarationNode {
public:
    std::string name;
    DataType returnType;
    int btabRef = -1;

    std::shared_ptr<BlockNode> body;

    FunctionDeclarationNode(
        const std::string& n,
        DataType rt
    )
        : DeclarationNode(ASTNodeType::FuncDecl),
          name(n),
          returnType(rt) {}
};

// Block: declarations + statements
class BlockNode : public ASTNode {
public:
    std::vector<std::shared_ptr<DeclarationNode>> declarations;
    std::shared_ptr<CompoundStatementNode> statements;

    BlockNode()
        : ASTNode(ASTNodeType::Block) {}
};

// Program Root
class ProgramNode : public ASTNode {
public:
    std::string name;

    std::vector<std::shared_ptr<DeclarationNode>> declarations;
    std::shared_ptr<BlockNode> mainBlock;

    ProgramNode(const std::string& n)
        : ASTNode(ASTNodeType::Program),
          name(n) {}
};

// Error Node
class ErrorNode : public ASTNode {
public:
    std::string message;

    ErrorNode(const std::string& msg)
        : ASTNode(ASTNodeType::Error),
          message(msg) {}
};