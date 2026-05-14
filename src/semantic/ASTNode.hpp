#pragma once
#include "../common/DataType.hpp"
#include "../common/NodeType.hpp"
#include <string>
#include <vector>


class ASTNode {
public:
    NodeType nodeType;
    int line;
    int col;

    ASTNode(NodeType t, int l = 0, int c = 0) : nodeType(t), line(l), col(c) {}
    virtual ~ASTNode() = default;
};


class ExpressionNode : public ASTNode {
public:
    // Tipe data hasil evaluasi (INTEGER, REAL, dll)
    DataType dataType; 
    
    ExpressionNode(NodeType t, DataType dt = DataType::UNKNOWN) 
        : ASTNode(t), dataType(dt) {}
};


class StatementNode : public ASTNode {
public:
    StatementNode(NodeType t) : ASTNode(t) {}
};


class DeclarationNode : public ASTNode {
public:
    DeclarationNode(NodeType t) : ASTNode(t) {}
};




class LiteralNode : public ExpressionNode {
public:
    std::string value;
    LiteralNode(DataType dt, std::string val) 
        : ExpressionNode(NodeType::Literal, dt) { value = val; }
};

class VariableNode : public ExpressionNode {
public:
    std::string name;
    int tabIndex; // Indeks ke 'tab' di SymbolTable
    VariableNode(std::string n) : ExpressionNode(NodeType::Variable) { name = n; }
};

class BinaryOpNode : public ExpressionNode {
public:
    std::shared_ptr<ExpressionNode> left;
    std::shared_ptr<ExpressionNode> right;
    std::string op; // +, -, *, /, DIV, MOD, dll
    
    BinaryOpNode(std::string o, std::shared_ptr<ExpressionNode> l, std::shared_ptr<ExpressionNode> r)
        : ExpressionNode(NodeType::BinaryOperation), op(o), left(l), right(r) {}
};

class AssignmentNode : public StatementNode {
public:
    std::shared_ptr<VariableNode> left;
    std::shared_ptr<ExpressionNode> right;
    
    AssignmentNode(std::shared_ptr<VariableNode> l, std::shared_ptr<ExpressionNode> r)
        : StatementNode(NodeType::AssignmentStatement), left(l), right(r) {}
};

class IfStatementNode : public StatementNode {
public:
    std::shared_ptr<ExpressionNode> condition;
    std::shared_ptr<ASTNode> thenPart;
    std::shared_ptr<ASTNode> elsePart; // Bisa nullptr

    IfStatementNode(std::shared_ptr<ExpressionNode> cond, std::shared_ptr<ASTNode> tp, std::shared_ptr<ASTNode> ep = nullptr)
        : StatementNode(NodeType::IfStatement), condition(cond), thenPart(tp), elsePart(ep) {}
};

class CompoundStatementNode : public StatementNode {
public:
    std::vector<std::shared_ptr<ASTNode>> statements;
    CompoundStatementNode() : StatementNode(NodeType::CompoundStatement) {}
};


class VarDeclarationNode : public DeclarationNode {
public:
    std::vector<std::string> idList;
    DataType type;
    int atabRef; // Jika array, simpan indeks ke atab di sini
    
    VarDeclarationNode(std::vector<std::string> ids, DataType dt)
        : DeclarationNode(NodeType::VarDeclaration), idList(ids), type(dt) {}
};

class ProcDeclarationNode : public DeclarationNode {
public:
    std::string name;
    int btabRef; // Prosedur punya blok sendiri di btab
    std::shared_ptr<ASTNode> block;
    
    ProcDeclarationNode(std::string n) : DeclarationNode(NodeType::ProcedureDeclaration), name(n) {}
};

class ErrorNode : public ASTNode {
public:
    std::string errorMessage;
    ErrorNode(std::string msg) : ASTNode(NodeType::Error), errorMessage(msg) {}
};