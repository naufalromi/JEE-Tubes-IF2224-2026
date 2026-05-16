#pragma once

class ProgramNode;
class BlockNode;

class SimpleTypeNode;
class ArrayTypeNode;
class RecordTypeNode;
class EnumeratedTypeNode;
class RangeTypeNode;

class ConstDeclarationNode;
class VarDeclarationNode;
class TypeDeclarationNode;
class ProcedureDeclarationNode;
class FunctionDeclarationNode;

class IntegerLiteralNode;
class RealLiteralNode;
class StringLiteralNode;
class BooleanLiteralNode;
class CharLiteralNode;

class VarAccessNode;
class ArrayAccessNode;
class FieldAccessNode;

class BinaryOpNode;
class UnaryOpNode;
class FunctionCallNode;

class CompoundStatementNode;
class AssignmentStatementNode;
class IfStatementNode;
class CaseStatementNode;
class WhileLoopNode;
class ForLoopNode;
class RepeatUntilNode;
class ProcedureCallNode;
class EmptyStatementNode;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(BlockNode* node) = 0;

    virtual void visit(SimpleTypeNode* node) = 0;
    virtual void visit(ArrayTypeNode* node) = 0;
    virtual void visit(RecordTypeNode* node) = 0;
    virtual void visit(EnumeratedTypeNode* node) = 0;
    virtual void visit(RangeTypeNode* node) = 0;

    virtual void visit(ConstDeclarationNode* node) = 0;
    virtual void visit(VarDeclarationNode* node) = 0;
    virtual void visit(TypeDeclarationNode* node) = 0;
    virtual void visit(ProcedureDeclarationNode* node) = 0;
    virtual void visit(FunctionDeclarationNode* node) = 0;

    virtual void visit(IntegerLiteralNode* node) = 0;
    virtual void visit(RealLiteralNode* node) = 0;
    virtual void visit(StringLiteralNode* node) = 0;
    virtual void visit(BooleanLiteralNode* node) = 0;
    virtual void visit(CharLiteralNode* node) = 0; 
    virtual void visit(VarAccessNode* node) = 0;
    virtual void visit(ArrayAccessNode* node) = 0;
    virtual void visit(FieldAccessNode* node) = 0;
    virtual void visit(BinaryOpNode* node) = 0;
    virtual void visit(UnaryOpNode* node) = 0;
    virtual void visit(FunctionCallNode* node) = 0;

    virtual void visit(CompoundStatementNode* node) = 0;
    virtual void visit(AssignmentStatementNode* node) = 0;
    virtual void visit(IfStatementNode* node) = 0;
    virtual void visit(CaseStatementNode* node) = 0;
    virtual void visit(WhileLoopNode* node) = 0;
    virtual void visit(ForLoopNode* node) = 0;
    virtual void visit(RepeatUntilNode* node) = 0;
    virtual void visit(ProcedureCallNode* node) = 0;
    virtual void visit(EmptyStatementNode* node) = 0;
};