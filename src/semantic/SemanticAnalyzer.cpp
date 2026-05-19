#include "SemanticAnalyzer.hpp"
#include "ASTPrinter.hpp"
#include <iomanip>
#include <iostream>

/**
 * Main semantic analysis pipeline:
 * ParseTree ->  ASTBuilder -> AST -> SemanticVisitor -> Decorated AST + Symbol Table
 */
std::shared_ptr<ProgramNode> SemanticAnalyzer::analyse()
{
    if (!parseTreeRoot) {
        std::cerr << "Error: Parse tree is null!\n";
        return nullptr;
    }

    try {
        // Build AST from parse tree
        builder = new ASTBuilder(parseTreeRoot);
        decoratedAST = builder->build();

        std::cout << "=======================================================\n";

        if (!decoratedAST) {
            std::cerr << "Error: Failed to build AST!\n";
            return nullptr;
        }

        // Apply semantic analysis (visitor pattern)
        // This decorates the AST and fills the symbol table
        decoratedAST->accept(&visitor);

        // Check for semantic errors
        if (visitor.hasErrors()) {
            std::cout << "\n[!] Semantic Analysis completed with errors:\n";
            visitor.printErrors();
        } else {
            std::cout << "[0] Semantic Analysis completed successfully!\n";
        }

        return decoratedAST;
    }
    catch (const std::exception& e) {
        std::cerr << "Error during semantic analysis: " << e.what() << "\n";
        return nullptr;
    }
}

/**
 * Print the decorated AST structure
 */
void SemanticAnalyzer::printDecoratedAST() const
{
    if (!decoratedAST) {
        std::cout << "Error: No decorated AST available!\n";
        return;
    }

    std::cout << "\n==================== DECORATED AST ====================\n";
    std::cout << "Program: " << decoratedAST->name << "\n";
    std::cout << "Scope Level: " << decoratedAST->scopeLevel << "\n";
    std::cout << "Line: " << decoratedAST->line << ", Column: " << decoratedAST->column << "\n";

    if (!decoratedAST->declarations.empty()) {
        std::cout << "\nDeclarations: " << decoratedAST->declarations.size() << "\n";
        for (size_t i = 0; i < decoratedAST->declarations.size(); i++) {
            auto& decl = decoratedAST->declarations[i];
            std::cout << "  [" << i << "] ";
            
            if (auto constDecl = std::dynamic_pointer_cast<ConstDeclarationNode>(decl)) {
                std::cout << "CONST " << constDecl->name << " (Line " << constDecl->line << ")\n";
            } else if (auto varDecl = std::dynamic_pointer_cast<VarDeclarationNode>(decl)) {
                std::cout << "VAR " << varDecl->name << " (Line " << varDecl->line << ")\n";
            } else if (auto typeDecl = std::dynamic_pointer_cast<TypeDeclarationNode>(decl)) {
                std::cout << "TYPE " << typeDecl->name << " (Line " << typeDecl->line << ")\n";
            } else if (auto procDecl = std::dynamic_pointer_cast<ProcedureDeclarationNode>(decl)) {
                std::cout << "PROCEDURE " << procDecl->name << " (Line " << procDecl->line << ")\n";
            } else if (auto funcDecl = std::dynamic_pointer_cast<FunctionDeclarationNode>(decl)) {
                std::cout << "FUNCTION " << funcDecl->name << " (Line " << funcDecl->line << ")\n";
            }
        }
    }
    
    std::cout << "===================== AST Generated ========================\n";
    ASTPrinter  astTree;
    astTree.print(decoratedAST);

    std::cout << "=======================================================\n\n";
}

/**
 * Print symbol table (tab)
 */
void SemanticAnalyzer::printSymbolTable() const
{
    const auto& tab = visitor.getSymbolTable().tab;

    std::cout << "\n==================== SYMBOL TABLE (TAB) ====================\n";
    std::cout << std::left << std::setw(7) << "Index"
              << std::setw(20) << "Name"
              << std::setw(15) << "Type"
              << std::setw(15) << "Object"
              << std::setw(8) << "Level"
              << std::setw(8) << "Link"
              << std::setw(8) << "Ref"
              << std::setw(6) << "Nrm"
              << std::setw(10) << "Adr/Val\n";
    std::cout << std::string(97, '-') << "\n";

    for (size_t i = 0; i < tab.size(); i++) {
        if (tab[i].name.empty()) continue; // Skip empty entries

        std::cout << std::left << std::setw(7) << i
                  << std::setw(20) << tab[i].name
                  << std::setw(15) << dataTypeToString(tab[i].type)
                  << std::setw(15) << objectTypeToString(tab[i].obj)
                  << std::setw(8) << tab[i].lev
                  << std::setw(8) << tab[i].link
                  << std::setw(8) << tab[i].ref
                  << std::setw(6) << tab[i].nrm
                  << std::setw(10) << tab[i].adr << "\n";
    }
    std::cout << "============================================================\n\n";
}

/**
 * Print array table (atab)
 */
void SemanticAnalyzer::printArrayTable() const
{
    const auto& atab = visitor.getSymbolTable().atab;

    std::cout << "\n==================== ARRAY TABLE (ATAB) ====================\n";
    std::cout << std::left << std::setw(8) << "Index"
              << std::setw(15) << "Index Type"
              << std::setw(15) << "Element Type"
              << std::setw(10) << "Elem Ref"
              << std::setw(8) << "Low"
              << std::setw(8) << "High"
              << std::setw(12) << "Elem Size"
              << std::setw(12) << "Total Size\n";
    std::cout << std::string(88, '-') << "\n";

    for (size_t i = 0; i < atab.size(); i++) {
        std::cout << std::left << std::setw(8) << i
                  << std::setw(15) << dataTypeToString(atab[i].xtyp)
                  << std::setw(15) << dataTypeToString(atab[i].etyp)
                  << std::setw(10) << atab[i].eref
                  << std::setw(8) << atab[i].low
                  << std::setw(8) << atab[i].high
                  << std::setw(12) << atab[i].elsz
                  << std::setw(12) << atab[i].size << "\n";
    }
    std::cout << "=============================================================\n\n";
}

/**
 * Print block table (btab)
 */
void SemanticAnalyzer::printBlockTable() const
{
    const auto& btab = visitor.getSymbolTable().btab;

    std::cout << "\n==================== BLOCK TABLE (BTAB) ====================\n";
    std::cout << std::left << std::setw(8) << "Index"
              << std::setw(8) << "Last"
              << std::setw(10) << "Last Param"
              << std::setw(12) << "Param Size"
              << std::setw(12) << "Var Size\n";
    std::cout << std::string(50, '-') << "\n";

    for (size_t i = 0; i < btab.size(); i++) {
        std::cout << std::left << std::setw(8) << i
                  << std::setw(8) << btab[i].last
                  << std::setw(10) << btab[i].lpar
                  << std::setw(12) << btab[i].psze
                  << std::setw(12) << btab[i].vsze << "\n";
    }
    std::cout << "============================================================\n\n";
}

/**
 * Print all semantic information
 */
void SemanticAnalyzer::printAllSemanticInfo() const
{
    std::cout << "          COMPLETE SEMANTIC ANALYSIS REPORT                 \n";

    // Print decorated AST
    printDecoratedAST();

    // Print symbol tables
    printSymbolTable();
    printArrayTable();
    printBlockTable();

    // Print errors if any
    if (visitor.hasErrors()) {
        std::cout << "\n==================== SEMANTIC ERRORS ====================\n";
        visitor.printErrors();
        std::cout << "=========================================================\n\n";
    }
}

SemanticAnalyzer::~SemanticAnalyzer()
{
    if (builder) {
        delete builder;
    }
}

