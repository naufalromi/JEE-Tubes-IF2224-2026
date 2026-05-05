
#pragma once

#include "../common/Token.hpp"
#include "../common/Tree.hpp"
#include "../common/NodeType.hpp"
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

class Parser {
public:
    std::vector<std::string> syntaxErrors;

	Parser(const std::vector<Token>& tokens);

	// First ever node
	std::shared_ptr<TreeNode> parseProgram();

	// production functions 
    std::shared_ptr<TreeNode> parseProgramHeader();          // <program-header>
    std::shared_ptr<TreeNode> parseDeclarationPart();        // <declaration-part>
    std::shared_ptr<TreeNode> parseConstDeclaration();       // <const-declaration>
    std::shared_ptr<TreeNode> parseConstant();               // <constant>
    std::shared_ptr<TreeNode> parseTypeDeclaration();        // <type-declaration>
    std::shared_ptr<TreeNode> parseVarDeclaration();         // <var-declaration>
    std::shared_ptr<TreeNode> parseIdentifierList();         // <identifier-list>
    std::shared_ptr<TreeNode> parseType();                   // <type>
    std::shared_ptr<TreeNode> parseArrayType();              // <array-type>
    std::shared_ptr<TreeNode> parseRange();                  // <range>
    std::shared_ptr<TreeNode> parseEnumerated();             // <enumerated>
    std::shared_ptr<TreeNode> parseRecordType();             // <record-type>
    std::shared_ptr<TreeNode> parseFieldList();              // <field-list>
    std::shared_ptr<TreeNode> parseFieldPart();              // <field-part>
    std::shared_ptr<TreeNode> parseSubprogramDeclaration();  // <subprogram-declaration>
    std::shared_ptr<TreeNode> parseProcedureDeclaration();   // <procedure-declaration>
    std::shared_ptr<TreeNode> parseFunctionDeclaration();    // <function-declaration>
    std::shared_ptr<TreeNode> parseBlock();                  // block
    std::shared_ptr<TreeNode> parseFormalParameterList();    // <formal-parameter-list>
    std::shared_ptr<TreeNode> parseParameterGroup();         // <parameter-group>
    std::shared_ptr<TreeNode> parseCompoundStatement();      // <compound-statement>
    std::shared_ptr<TreeNode> parseStatementList();          // <statement-list>
    std::shared_ptr<TreeNode> parseStatement();              // <statement>
    std::shared_ptr<TreeNode> parseAssignmentStatement();    // <assignment-statement>
    std::shared_ptr<TreeNode> parseIfStatement();            // <if-statement>
    std::shared_ptr<TreeNode> parseCaseStatement();          // <case-statement>
    std::shared_ptr<TreeNode> parseCaseBlock();              // <case-block>
    std::shared_ptr<TreeNode> parseWhileStatement();         // <while-statement>
    std::shared_ptr<TreeNode> parseRepeatStatement();        // <repeat-statement>
    std::shared_ptr<TreeNode> parseForStatement();           // <for-statement>
    std::shared_ptr<TreeNode> parseProcedureCall();          // <procedure/function-call>
    std::shared_ptr<TreeNode> parseParameterList();          // <parameter-list>
    std::shared_ptr<TreeNode> parseExpression();             // <expression>
    std::shared_ptr<TreeNode> parseSimpleExpression();       // <simple-expression>
    std::shared_ptr<TreeNode> parseTerm();                   // <term>
    std::shared_ptr<TreeNode> parseFactor();                 // <factor>
    std::shared_ptr<TreeNode> parseRelationalOperator();     // <relational-operator>
    std::shared_ptr<TreeNode> parseAdditiveOperator();       // <additive-operator>
    std::shared_ptr<TreeNode> parseMultiplicativeOperator(); // <multiplicative-operator>
    // Revisi
    std::shared_ptr<TreeNode> parseVariable();                // <variable>
    std::shared_ptr<TreeNode> parseIndexList();               // <index-list>
    std::shared_ptr<TreeNode> parseComponentVariable();       // <component-variable>

	const Token& peek(int offset = 0) const;
	const Token& advance();
    bool match(TokenType type) const;
    std::shared_ptr<TreeNode> expect(TokenType tokenType, NodeType nodeType, std::string expectedStr);

    size_t save() const;
    void restore(size_t saved);

    std::shared_ptr<TreeNode> fail(size_t saved);
    std::shared_ptr<TreeNode> terminal(TokenType tokenType, NodeType nodeType);

    bool need(const std::shared_ptr<TreeNode>& parent, const std::shared_ptr<TreeNode>& child);

    static std::string printNode(const std::shared_ptr<TreeNode> &node);
    static void printParseTree(const std::shared_ptr<TreeNode> &node, const std::string &prefix , bool last, bool root );

	const std::vector<Token>& tokens;
	size_t pos;
};

