#pragma once

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include "ASTNode.hpp" 
#include "ASTVisitor.hpp"
#include "SymbolTable.hpp"
#include "../common/Error.hpp"


/**
 * Implementasi Konkrit Semantic Visitor
 */
class SemanticVisitor : public ASTVisitor {
private:
    SymbolTable symbolTable;
    int currentLevel = 0;
    std::vector<SemanticError> errors;

    // Helper: Penanganan Error
    void reportError(ASTNode* node, const std::string& message);

    // Helper: Pengecekan Tipe Data
    bool isCompatible(DataType target, DataType source);
    DataType resolveBinaryType(const std::string& op, DataType left, DataType right);
    
    // Helper: Alokasi Memori
    int getDataTypeSize(DataType type);

public:
    SemanticVisitor() = default;

    // Mendapatkan hasil analisis
    bool hasErrors() const { return !errors.empty(); }
    void printErrors() const;
    SymbolTable& getSymbolTable() { return symbolTable; }
    const SymbolTable& getSymbolTable() const { return symbolTable; }

    // --- Implementasi Visit ---

    // Root & Scope
    void visit(ProgramNode* node) override;
    void visit(BlockNode* node) override;

    // Deklarasi (Mengisi Symbol Table)
    void visit(ConstDeclarationNode* node) override;
    void visit(VarDeclarationNode* node) override;
    void visit(TypeDeclarationNode* node) override;
    void visit(ProcedureDeclarationNode* node) override;
    void visit(FunctionDeclarationNode* node) override;

    // Tipe Data
    void visit(SimpleTypeNode* node) override;
    void visit(ArrayTypeNode* node) override;
    void visit(RecordTypeNode* node) override;
    void visit(EnumeratedTypeNode* node) override;
    void visit(RangeTypeNode* node) override;

    // Ekspresi (Inferensi Tipe & Dekorasi)
    void visit(IntegerLiteralNode* node) override;
    void visit(RealLiteralNode* node) override;
    void visit(StringLiteralNode* node) override;
    void visit(CharLiteralNode* node) override;
    void visit(BooleanLiteralNode* node) override;
    void visit(VarAccessNode* node) override;
    void visit(ArrayAccessNode* node) override;
    void visit(FieldAccessNode* node) override;
    void visit(BinaryOpNode* node) override;
    void visit(UnaryOpNode* node) override;
    void visit(FunctionCallNode* node) override;

    // Statement (Validasi Logika Alur)
    void visit(CompoundStatementNode* node) override;
    void visit(AssignmentStatementNode* node) override;
    void visit(IfStatementNode* node) override;
    void visit(CaseStatementNode* node) override;
    void visit(WhileLoopNode* node) override;
    void visit(ForLoopNode* node) override;
    void visit(RepeatUntilNode* node) override;
    void visit(ProcedureCallNode* node) override;
    void visit(EmptyStatementNode* node) override;
};
