#pragma once
#include <vector>
#include "../common/Error.hpp"
#include "../common/Tree.hpp"
#include "ASTNode.hpp"
#include "SymbolTable.hpp"
class SemanticAnalyzer
{
private:
    std::vector<SemanticError> errorList;
    std::shared_ptr<TreeNode> parseTreeRoot;


    SymbolTable symbolTable;
    int currentLevel = 0;

    void reportError(int line, int col, std::string msg);

public:
    SemanticAnalyzer(std::shared_ptr<TreeNode> root) : parseTreeRoot(root) {}

    /**
     * Fungsi ini akan melakukan Semantic Analysis terhadap ParseTree yang sudah dibuat
     */
    std::shared_ptr<ASTNode> analyse();

    /**
     * Melakukan DFS dengan prinsip L-Atrributed
     * Mengembalikan ASTNode dasar (polymorphism).
     */
    std::shared_ptr<ASTNode> visit(std::shared_ptr<TreeNode> node);


    /**
     * Kelompok Deklarasi Tabel berdasarkan aturan produksi grammar di sini akan mengisi 
     * tab, atab, btab dan bertujuan untuk pendaftaran identitas, alokasi memori 
     * dan menejemen scope 
     */

    /* # Struktur Program */
    std::shared_ptr<ASTNode>  visitProgram(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode>  visitProgramHeader(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode>  visitDeclarationPart(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode>  visitBlock(std::shared_ptr<TreeNode> node);

    /* ## Definisi Data */
    std::shared_ptr<DeclarationNode> visitConstDeclaration(std::shared_ptr<TreeNode> node);
    std::shared_ptr<DeclarationNode> visitTypeDeclaration(std::shared_ptr<TreeNode> node);
    std::shared_ptr<DeclarationNode> visitVarDeclaration(std::shared_ptr<TreeNode> node);
    
    std::shared_ptr<std::string> visitIdentifierList(std::shared_ptr<TreeNode> node);
    
    /* ## Kontruksi Tipe untuk mengisi atab/btab */
    DataType visitType(std::shared_ptr<TreeNode> node);
    DataType visitArrayType(std::shared_ptr<TreeNode> node);
    DataType visitEnumerated(std::shared_ptr<TreeNode> node);
    DataType visitRecordType(std::shared_ptr<TreeNode> node);
    
    void visitRange(std::shared_ptr<TreeNode> node);
    void visitFieldList(std::shared_ptr<TreeNode> node);
    void visitFieldPart(std::shared_ptr<TreeNode> node);

    /* ## Subprogram */
    std::shared_ptr<DeclarationNode> visitSubprogramDeclaration(std::shared_ptr<TreeNode> node);
    std::shared_ptr<DeclarationNode> visitProcedureDeclaration(std::shared_ptr<TreeNode> node);
    std::shared_ptr<DeclarationNode> visitFunctionDeclaration(std::shared_ptr<TreeNode> node);
    
    std::vector<std::shared_ptr<DeclarationNode>> visitParameterGroup(std::shared_ptr<TreeNode> node);
    std::vector<std::shared_ptr<DeclarationNode>> visitFormalParameterList(std::shared_ptr<TreeNode> node);
    /** #  Definisi Expression 
     * Output dari visit disini harus berupa DataType yang sesuai dan juga  mengecek Kapabilitas tipe data
     * Penjumlahan antar integer menghasilkan integer, penjumlahan integer dan float menghasilkan float
     * Real tidak bisa di MOD misalnya
    */

    /* ## Heirarki Operasi */
    std::shared_ptr<ExpressionNode> visitExpression(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ExpressionNode> visitSimpleExpression(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ExpressionNode> visitTerm(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ExpressionNode> visitFactor(std::shared_ptr<TreeNode> node);

    /* ## Operand dan Akses Data */
    std::shared_ptr<ExpressionNode> visitConstant(std::shared_ptr<TreeNode> node);
    std::shared_ptr<VariableNode> visitVariable(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ExpressionNode> visitComponentVariable(std::shared_ptr<TreeNode> node);
    
    std::vector<std::shared_ptr<ExpressionNode>> visitIndexList(std::shared_ptr<TreeNode> node);

    /* ## Operator Rule */
    void visitRelationalOperator(std::shared_ptr<TreeNode> node);
    void visitAdditiveOperator(std::shared_ptr<TreeNode> node);
    void visitMultiplicativeOperator(std::shared_ptr<TreeNode> node);

    /**
     * # Kelompok Statement
     * Menghasilkan tipe data VOID hanya untuk mengecek intruksi yang ditulis masuk akal
     */
    
    /* ## Blok eksekusi */
    std::shared_ptr<ASTNode> visitCompoundStatement(std::shared_ptr<TreeNode> node );
    std::shared_ptr<ASTNode> visitStatementList(std::shared_ptr<TreeNode> node );
    std::shared_ptr<ASTNode> visitStatement(std::shared_ptr<TreeNode> node );

    /* ## Kontrol Intruksi */
    std::shared_ptr<ASTNode> visitAssignmentStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode> visitIfStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode> visitCaseStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode> visitCaseBlock(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode> visitWhileStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode> visitRepeatStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode> visitForStatement(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode> visitProcedureFunctionCall(std::shared_ptr<TreeNode> node);
    std::shared_ptr<ASTNode> visitParameterList(std::shared_ptr<TreeNode> node);
    
    



    
    
    bool isTypeCompatible(DataType target, DataType source);
    bool checkFlowLogic(std::shared_ptr<ASTNode> root);
    void showErrors();
    void printAST(std::shared_ptr<ASTNode> node, std::string prefix = "", bool isLeft = true, bool isRoot = true);
    ~SemanticAnalyzer();
};

