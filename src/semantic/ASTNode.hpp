#pragma once

#include "ASTVisitor.hpp"
#include "../common/DataType.hpp"
#include <memory>
#include <string>
#include <vector>

// 1. AST Node Types (Fokus pada Struktur Atas & Deklarasi)
enum class ASTNodeType {
    Program, BlockNode,
    // Declaration
    ConstDecl, VarDecl, TypeDecl, ProcDecl, FuncDecl,
    // Data Type
    SimpleType, ArrayType, RecordType, EnumeratedType, RangeType,
    // Expression
    IntLiteral, RealLiteral, StringLiteral, CharLiteral, BoolLiteral, VarAccess, ArrayAccess, FieldAccess, BinaryOp, UnaryOp, FuncCall,
    // Statement
    CompoundStatement, AssignmentStatement, IfStatement, CaseStatement, ForStatement, WhileStatement, RepeatUntil, ProcedureCall, EmptyStatement
};

class ExpressionNode; 

// Base Class ASTNode
class ASTNode {
public:
    ASTNodeType nodeType;
    int scopeLevel = -1;
    std::vector<std::string> semanticErrors;

    int line;
    int column;

    ASTNode(ASTNodeType t) : nodeType(t) {}
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor* visitor) = 0;
};

/**
 * DATA TYPE
 */
class TypeNode : public ASTNode {
public:
    DataType resolvedType = DataType::UNKNOWN;
    TypeNode(ASTNodeType t) : ASTNode(t) {}
};

// Tipe Dasar (misal: "integer", "real", "boolean", atau nama alias custom)
class SimpleTypeNode : public TypeNode {
public:
    std::string name;

    SimpleTypeNode(const std::string& n) : TypeNode(ASTNodeType::SimpleType), name(n) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// Tipe Array (misal: array [1..10] of integer)
class ArrayTypeNode : public TypeNode {
public:
    std::shared_ptr<TypeNode> indexType;
    std::shared_ptr<TypeNode> elementType; 

    ArrayTypeNode(std::shared_ptr<TypeNode> idxType, std::shared_ptr<TypeNode> elemType)
        : TypeNode(ASTNodeType::ArrayType), indexType(idxType), elementType(elemType) {}
    
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// Tipe Record (misal: record x: integer; y: real; end)
class RecordTypeNode : public TypeNode {
public:
    std::vector<std::shared_ptr<VarDeclarationNode>> fields;

    RecordTypeNode() : TypeNode(ASTNodeType::RecordType) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// Tipe Enumerated (misal: (Senin, Selasa, Rabu))
class EnumeratedTypeNode : public TypeNode {
public:
    std::vector<std::string> elements;
    EnumeratedTypeNode() : TypeNode(ASTNodeType::EnumeratedType) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// Tipe Range (misal: 1..100)
class RangeTypeNode : public TypeNode {
public:
    std::shared_ptr<ExpressionNode> lowBound;
    std::shared_ptr<ExpressionNode> highBound;
    RangeTypeNode(std::shared_ptr<ExpressionNode> low, std::shared_ptr<ExpressionNode> high) 
        : TypeNode(ASTNodeType::RangeType), lowBound(low), highBound(high) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

/**
 * DECLARATION
 */
class DeclarationNode : public ASTNode {
public:
    DeclarationNode(ASTNodeType t) : ASTNode(t) {}
};

// CONST: Nama = Nilai
class ConstDeclarationNode : public DeclarationNode {
public:
    std::string name;
    std::shared_ptr<ExpressionNode> value;

    ConstDeclarationNode(const std::string& n, std::shared_ptr<ExpressionNode> val)
        : DeclarationNode(ASTNodeType::ConstDecl), name(n), value(val) {}
    
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// VAR: Nama = Tipe
class VarDeclarationNode : public DeclarationNode {
public:
    std::string name;
    std::shared_ptr<TypeNode> typeDefinition;

    VarDeclarationNode(const std::string& n, std::shared_ptr<TypeNode> typeDef)
        : DeclarationNode(ASTNodeType::VarDecl), name(n), typeDefinition(typeDef) {}

    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// TYPE: Nama = Struktur Tipe
class TypeDeclarationNode : public DeclarationNode {
public:
    std::string name;
    std::shared_ptr<TypeNode> typeDefinition;

    TypeDeclarationNode(const std::string& n, std::shared_ptr<TypeNode> typeDef)
        : DeclarationNode(ASTNodeType::TypeDecl), name(n), typeDefinition(typeDef) {}

    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// PROCEDURE: Nama = Parameter + Block
class ProcedureDeclarationNode : public DeclarationNode {
public:
    std::string name;
    std::vector<std::shared_ptr<VarDeclarationNode>> parameters;
    std::shared_ptr<BlockNode> body;
    int localVariablesSize = 0;

    ProcedureDeclarationNode(const std::string& n) 
        : DeclarationNode(ASTNodeType::ProcDecl), name(n) {}

    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// FUNCTION: Nama = Parameter + Return Type + Block
class FunctionDeclarationNode : public DeclarationNode {
public:
    std::string name;
    std::vector<std::shared_ptr<VarDeclarationNode>> parameters;
    std::shared_ptr<TypeNode> returnType;
    std::shared_ptr<BlockNode> body;
    int localVariablesSize = 0;

    FunctionDeclarationNode(const std::string& n, std::shared_ptr<TypeNode> retType) 
        : DeclarationNode(ASTNodeType::FuncDecl), name(n), returnType(retType) {}

    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

/**
 * EXPRESSION
 */
class ExpressionNode : public ASTNode {
public:
    // Semantic Analyzer akan mengisi ini nanti)
    DataType evaluatedType = DataType::UNKNOWN;

    ExpressionNode(ASTNodeType t) : ASTNode(t) {}
};

class IntegerLiteralNode : public ExpressionNode {
public:
    int value;
    IntegerLiteralNode(int val) : ExpressionNode(ASTNodeType::IntLiteral), value(val) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class RealLiteralNode : public ExpressionNode {
public:
    double value;
    RealLiteralNode(double val) : ExpressionNode(ASTNodeType::RealLiteral), value(val) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class StringLiteralNode : public ExpressionNode {
public:
    std::string value;
    StringLiteralNode(const std::string& val) : ExpressionNode(ASTNodeType::StringLiteral), value(val) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class CharLiteralNode : public ExpressionNode {
public:
    char value;
    CharLiteralNode(char val) : ExpressionNode(ASTNodeType::CharLiteral), value(val) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class BooleanLiteralNode : public ExpressionNode {
public:
    bool value;
    BooleanLiteralNode(bool val) : ExpressionNode(ASTNodeType::BoolLiteral), value(val) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class VarAccessNode : public ExpressionNode {
public:
    std::string name;
    bool isConstant = false;
    std::weak_ptr<DeclarationNode> declarationRef; 

    VarAccessNode(const std::string& n) : ExpressionNode(ASTNodeType::VarAccess), name(n) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class ArrayAccessNode : public ExpressionNode {
public:
    std::shared_ptr<ExpressionNode> target;
    std::shared_ptr<ExpressionNode> index;  

    ArrayAccessNode(std::shared_ptr<ExpressionNode> tgt, std::shared_ptr<ExpressionNode> idx)
        : ExpressionNode(ASTNodeType::ArrayAccess), target(tgt), index(idx) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class FieldAccessNode : public ExpressionNode {
public:
    std::shared_ptr<ExpressionNode> target; 
    std::string fieldName;                  

    FieldAccessNode(std::shared_ptr<ExpressionNode> tgt, const std::string& field)
        : ExpressionNode(ASTNodeType::FieldAccess), target(tgt), fieldName(field) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class BinaryOpNode : public ExpressionNode {
public:
    std::string op;
    std::shared_ptr<ExpressionNode> left;
    std::shared_ptr<ExpressionNode> right;

    BinaryOpNode(const std::string& o, std::shared_ptr<ExpressionNode> l, std::shared_ptr<ExpressionNode> r)
        : ExpressionNode(ASTNodeType::BinaryOp), op(o), left(l), right(r) {}
        
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class UnaryOpNode : public ExpressionNode {
public:
    std::string op;
    std::shared_ptr<ExpressionNode> operand;

    UnaryOpNode(const std::string& o, std::shared_ptr<ExpressionNode> expr)
        : ExpressionNode(ASTNodeType::UnaryOp), op(o), operand(expr) {}
        
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class FunctionCallNode : public ExpressionNode {
public:
    std::string name; 
    std::vector<std::shared_ptr<ExpressionNode>> args; 
    
    std::weak_ptr<DeclarationNode> declarationRef; 

    FunctionCallNode(const std::string& n) : ExpressionNode(ASTNodeType::FuncCall), name(n) {}
    
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

/**
 * STATEMENT
 */
class StatementNode : public ASTNode {
public:
    StatementNode(ASTNodeType type) : ASTNode(type) {}
};

// COMPOUND STATEMENT (begin ... end)
class CompoundStatementNode : public StatementNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements;

    CompoundStatementNode() : StatementNode(ASTNodeType::CompoundStatement) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// ASSIGNMENT STATEMENT (:=)
class AssignmentStatementNode : public StatementNode {
public:
    std::shared_ptr<ExpressionNode> target;
    std::shared_ptr<ExpressionNode> value;

    AssignmentStatementNode(std::shared_ptr<ExpressionNode> tgt, std::shared_ptr<ExpressionNode> val)
        : StatementNode(ASTNodeType::AssignmentStatement), target(tgt), value(val) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// IF STATEMENT (if ... then ... else)
class IfStatementNode : public StatementNode {
public:
    std::shared_ptr<ExpressionNode> condition;
    std::shared_ptr<StatementNode> thenBranch;
    std::shared_ptr<StatementNode> elseBranch;

    IfStatementNode(std::shared_ptr<ExpressionNode> cond, std::shared_ptr<StatementNode> thenBr, std::shared_ptr<StatementNode> elseBr = nullptr)
        : StatementNode(ASTNodeType::IfStatement), condition(cond), thenBranch(thenBr), elseBranch(elseBr) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// CASE STATEMENT (case ... of ... end)
class CaseStatementNode : public StatementNode {
public:
    std::shared_ptr<ExpressionNode> expression;
    std::vector<std::pair<std::vector<std::shared_ptr<ExpressionNode>>, std::shared_ptr<StatementNode>>> cases;
    std::shared_ptr<StatementNode> elseBranch; // Nullable

    CaseStatementNode(std::shared_ptr<ExpressionNode> expr)
        : StatementNode(ASTNodeType::CaseStatement), expression(expr), elseBranch(nullptr) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// WHILE LOOP (while ... do)
class WhileLoopNode : public StatementNode {
public:
    std::shared_ptr<ExpressionNode> condition;
    std::shared_ptr<StatementNode> body;

    WhileLoopNode(std::shared_ptr<ExpressionNode> cond, std::shared_ptr<StatementNode> b)
        : StatementNode(ASTNodeType::WhileStatement), condition(cond), body(b) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// FOR LOOP (for ... to/downto ... do)
class ForLoopNode : public StatementNode {
public:
    std::string counterVar;
    std::shared_ptr<ExpressionNode> startValue;
    std::shared_ptr<ExpressionNode> endValue;
    bool isDownTo; // false untuk 'to', true untuk 'downto'
    std::shared_ptr<StatementNode> body;

    ForLoopNode(const std::string& var, std::shared_ptr<ExpressionNode> start, std::shared_ptr<ExpressionNode> end, bool downTo, std::shared_ptr<StatementNode> b)
        : StatementNode(ASTNodeType::ForStatement), counterVar(var), startValue(start), endValue(end), isDownTo(downTo), body(b) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// REPEAT UNTIL LOOP (repeat ... until)
class RepeatUntilNode : public StatementNode {
public:
    std::vector<std::shared_ptr<StatementNode>> body;
    std::shared_ptr<ExpressionNode> condition;

    RepeatUntilNode(std::shared_ptr<ExpressionNode> cond)
        : StatementNode(ASTNodeType::RepeatUntil), condition(cond) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// PROCEDURE CALL STATEMENT
class ProcedureCallNode : public StatementNode {
public:
    std::string name;
    std::vector<std::shared_ptr<ExpressionNode>> args;

    ProcedureCallNode(const std::string& n)
        : StatementNode(ASTNodeType::ProcedureCall), name(n) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// EMPTY STATEMENT
class EmptyStatementNode : public StatementNode {
public:
    EmptyStatementNode() : StatementNode(ASTNodeType::EmptyStatement) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class BlockNode : public ASTNode {
public:
    std::vector<std::shared_ptr<DeclarationNode>> declarations;
    std::shared_ptr<CompoundStatementNode> statements;

    BlockNode() : ASTNode(ASTNodeType::BlockNode) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class ProgramNode : public ASTNode {
public:
    std::string name;
    std::vector<std::shared_ptr<DeclarationNode>> declarations;
    std::shared_ptr<CompoundStatementNode> statements;

    ProgramNode(const std::string& n) : ASTNode(ASTNodeType::Program), name(n) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};