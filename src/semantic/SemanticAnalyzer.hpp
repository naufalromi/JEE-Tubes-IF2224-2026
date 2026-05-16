#pragma once
#include <vector>
#include "../common/Error.hpp"
#include "../common/Tree.hpp"
#include "ASTNode.hpp"
#include "ASTBuilder.hpp"
#include "SemanticVisitor.hpp"
#include "SymbolTable.hpp"

class SemanticAnalyzer
{
private:
    std::shared_ptr<TreeNode> parseTreeRoot;
    std::shared_ptr<ProgramNode> decoratedAST;
    SemanticVisitor visitor;
    ASTBuilder* builder;

public:
    SemanticAnalyzer(std::shared_ptr<TreeNode> root) 
        : parseTreeRoot(root), decoratedAST(nullptr), visitor(), builder(nullptr) {}

    /**
     * Main analysis function: Parse Tree → AST → Decorated AST
     * Combines ASTBuilder and SemanticVisitor in one pipeline
     */
    std::shared_ptr<ProgramNode> analyse();

    /**
     * Print the decorated AST
     */
    void printDecoratedAST() const;

    /**
     * Print symbol table (tab)
     */
    void printSymbolTable() const;

    /**
     * Print array table (atab)
     */
    void printArrayTable() const;

    /**
     * Print block table (btab)
     */
    void printBlockTable() const;

    /**
     * Print all semantic information
     */
    void printAllSemanticInfo() const;

    /**
     * Get semantic errors
     */
    bool hasErrors() const { return visitor.hasErrors(); }
    void printErrors() const { visitor.printErrors(); }

    /**
     * Get decorated AST
     */
    std::shared_ptr<ProgramNode> getDecoratedAST() const { return decoratedAST; }

    /**
     * Get symbol table
     */
    SymbolTable& getSymbolTable() { return visitor.getSymbolTable(); }

    ~SemanticAnalyzer();
};

