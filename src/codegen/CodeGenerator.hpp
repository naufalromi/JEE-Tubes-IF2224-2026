#pragma once

#include "../semantic/ASTVisitor.hpp"
#include "../semantic/SymbolTable.hpp"
#include "../common/Instruction.hpp"
#include <vector>
#include <unordered_map>

class CodeGenerator : public ASTVisitor {
private:
    std::vector<Instruction> instructions;
    const SymbolTable* symbolTable;
    std::unordered_map<int, int> subroutineAddresses;
    std::vector<std::pair<int, int>> unresolvedCalls;

public:
    CodeGenerator(const SymbolTable* symTab);
    virtual ~CodeGenerator() = default;

    void generate(ProgramNode* root);
    const std::vector<Instruction>& getInstructions() const;

private:
    void emit(OpCode op, int level, int operand);
    void emitLiteral(int level, RuntimeValue value);
    int getNextAddress() const;
    void backpatch(int instructionIndex, int targetAddress);
    void resolveFunctionCalls();

public:
    bool exportToFile(const std::string& filename) const;

    void visit(ProgramNode* node) override;
    void visit(BlockNode* node) override;

    void visit(SimpleTypeNode* node) override;
    void visit(ArrayTypeNode* node) override;
    void visit(RecordTypeNode* node) override;
    void visit(EnumeratedTypeNode* node) override;
    void visit(RangeTypeNode* node) override;

    void visit(ConstDeclarationNode* node) override;
    void visit(VarDeclarationNode* node) override;
    void visit(TypeDeclarationNode* node) override;
    void visit(ProcedureDeclarationNode* node) override;
    void visit(FunctionDeclarationNode* node) override;

    void visit(IntegerLiteralNode* node) override;
    void visit(RealLiteralNode* node) override;
    void visit(StringLiteralNode* node) override;
    void visit(BooleanLiteralNode* node) override;
    void visit(CharLiteralNode* node) override;
    
    void visit(VarAccessNode* node) override;
    void visit(ArrayAccessNode* node) override;
    void visit(FieldAccessNode* node) override;
    
    void visit(BinaryOpNode* node) override;
    void visit(UnaryOpNode* node) override;
    void visit(FunctionCallNode* node) override;

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