#pragma once

#include "ASTNode.hpp"
#include "../common/Tree.hpp"
#include <memory>
#include <vector>

class ASTBuilder {
private:
    std::shared_ptr<TreeNode> parseTreeRoot;

    std::shared_ptr<ProgramNode> buildProgram(std::shared_ptr<TreeNode> node);
    std::shared_ptr<BlockNode> buildBlock(std::shared_ptr<TreeNode> node);

    // Declaration
    std::vector<std::shared_ptr<DeclarationNode>> buildDeclarationPart(std::shared_ptr<TreeNode> node);
    std::vector<std::shared_ptr<ConstDeclarationNode>> buildConstDeclarations(std::shared_ptr<TreeNode> node);
    std::vector<std::shared_ptr<VarDeclarationNode>> buildVarDeclarations(std::shared_ptr<TreeNode> node);
    std::vector<std::shared_ptr<TypeDeclarationNode>> buildTypeDeclarations(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ProcedureDeclarationNode> buildProcedureDeclaration(std::shared_ptr<TreeNode> node);
    std::shared_ptr<FunctionDeclarationNode> buildFunctionDeclaration(std::shared_ptr<TreeNode> node);
    std::vector<std::shared_ptr<VarDeclarationNode>> buildFormalParameters(std::shared_ptr<TreeNode> node);

    // Data Type
    std::shared_ptr<TypeNode> buildType(std::shared_ptr<TreeNode> node);
    std::shared_ptr<TypeNode> buildRecordType(std::shared_ptr<TreeNode> node);
    std::shared_ptr<TypeNode> buildArrayType(std::shared_ptr<TreeNode> node);
    std::shared_ptr<TypeNode> buildRangeType(std::shared_ptr<TreeNode> node);
    std::shared_ptr<TypeNode> buildEnumeratedType(std::shared_ptr<TreeNode> node);

    // Expression
    std::shared_ptr<ExpressionNode> buildExpression(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ExpressionNode> buildLiteral(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ExpressionNode> buildVariableAccess(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ExpressionNode> buildFunctionCall(std::shared_ptr<TreeNode> node);

    // Statement
    std::shared_ptr<StatementNode> buildStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<CompoundStatementNode> buildCompoundStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<AssignmentStatementNode> buildAssignmentStatement(std::shared_ptr<TreeNode> node); 
    std::shared_ptr<IfStatementNode> buildIfStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<CaseStatementNode> buildCaseStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<WhileLoopNode> buildWhileLoop(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ForLoopNode> buildForLoop(std::shared_ptr<TreeNode> node);
    std::shared_ptr<RepeatUntilNode> buildRepeatUntil(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ProcedureCallNode> buildProcedureCall(std::shared_ptr<TreeNode> node);

public:
    ASTBuilder(std::shared_ptr<TreeNode> root) : parseTreeRoot(root) {}

    std::shared_ptr<ProgramNode> build() {
        if (!parseTreeRoot) return nullptr;
        return buildProgram(parseTreeRoot);
    }
};