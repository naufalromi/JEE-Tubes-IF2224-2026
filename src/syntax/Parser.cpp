
#include "Parser.hpp"
#include "iostream"


Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), pos(0) {}

bool Parser::match(TokenType type) const {
    return pos < tokens.size() && tokens[pos].type == type;
}

size_t Parser::save() const {
    return pos;
}

void Parser::restore(size_t saved) {
    pos = saved;
}

std::shared_ptr<TreeNode> Parser::fail(size_t saved) {
    restore(saved);
    return nullptr;
}

std::shared_ptr<TreeNode> Parser::terminal(TokenType tokenType, NodeType nodeType) {
    if (!match(tokenType)) {
        return nullptr;
    }

    auto value = tokens[pos].value;
    pos++;

    return std::make_shared<TreeNode>(nodeType, value);
}

std::shared_ptr<TreeNode> Parser::expect(TokenType tokenType, NodeType nodeType, std::string expectedStr) {
    if (match(tokenType)) {
        auto value = tokens[pos].value;
        pos++;
        return std::make_shared<TreeNode>(nodeType, value);
    }

    Token errorToken =  (pos < tokens.size()) ? tokens[pos] : tokens.back();
    std::string errMsg = "Syntax Error at line " + std::to_string(errorToken.line) + ", column " + std::to_string(errorToken.column) + ": Expected '" + expectedStr + "', but got '" + errorToken.value + "'";
    syntaxErrors.push_back(errMsg);
    return std::make_shared<TreeNode>(nodeType, "<missing " + expectedStr + ">");
}

// this function checks wether the child exist or not
bool Parser::need(const std::shared_ptr<TreeNode>& parent, const std::shared_ptr<TreeNode>& child) {
    if (child == nullptr) {
        return false;
    }

    parent->addChild(child);
    return true;
}

//1
std::shared_ptr<TreeNode> Parser::parseProgram() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Program);

    if (!need(node, parseProgramHeader())) return fail(saved);
    if (!need(node, parseDeclarationPart())) return fail(saved);
    if (!need(node, parseCompoundStatement())) return fail(saved);
    need(node, expect(TokenType::PERIOD, NodeType::Period, "."));

    return node;
}

//2
std::shared_ptr<TreeNode> Parser::parseProgramHeader() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ProgramHeader);

    if (!need(node, terminal(TokenType::PROGRAMSY, NodeType::ProgramSy))) return fail(saved);
    need(node, expect(TokenType::IDENT, NodeType::Ident, "program identifier"));
    need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));

    return node;
}

//3
std::shared_ptr<TreeNode> Parser::parseDeclarationPart() {
    auto node = std::make_shared<TreeNode>(NodeType::DeclarationPart);

    // a* + b* + c* + d*
    while (need(node, parseConstDeclaration()));
    while (need(node, parseTypeDeclaration()));
    while (need(node, parseVarDeclaration()));
    while (need(node, parseSubprogramDeclaration()));

    return node;
}

//4
std::shared_ptr<TreeNode> Parser::parseConstDeclaration() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ConstDeclaration);

    if (!need(node, terminal(TokenType::CONSTSY, NodeType::ConstSy))) return fail(saved);

    do {
        if (!need(node, terminal(TokenType::IDENT, NodeType::Ident))) return fail(saved);
        need(node, expect(TokenType::EQL, NodeType::Eql, "=="));
        if (!need(node, parseConstant())) return fail(saved);
        need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));
    } while (match(TokenType::IDENT));

    return node;
}

//5
std::shared_ptr<TreeNode> Parser::parseConstant() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Constant);

    if (need(node, terminal(TokenType::CHARCON, NodeType::CharCon))) return node;
    restore(saved);

    if (need(node, terminal(TokenType::STRINGCON, NodeType::String))) return node;
    restore(saved);

    // (plus | minus)?
    need(node, terminal(TokenType::PLUS, NodeType::Plus)) || 
    need(node, terminal(TokenType::MINUS, NodeType::Minus));

    // ident | intcon | realcon
    if (need(node, terminal(TokenType::IDENT, NodeType::Ident)) || 
    need(node, terminal(TokenType::INTCON, NodeType::IntCon)) || 
    need(node, terminal(TokenType::REALCON, NodeType::RealCon))) return node;
    
    return fail(saved);
}

//6
std::shared_ptr<TreeNode> Parser::parseTypeDeclaration() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::TypeDeclaration);

    if (!need(node, terminal(TokenType::TYPESY, NodeType::TypeSy))) return fail(saved);

    do {
        if(!need(node, terminal(TokenType::IDENT, NodeType::Ident))) return fail(saved);
        need(node, expect(TokenType::EQL, NodeType::Eql, "=="));
        if(!need(node, parseType())) return fail(saved);
        need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));
    } while (match(TokenType::IDENT));

    return node;
}

//7
std::shared_ptr<TreeNode> Parser::parseVarDeclaration() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::VarDeclaration);

    if (!need(node, terminal(TokenType::VARSY, NodeType::VarSy))) return fail(saved);

    do {
        if (!need(node, parseIdentifierList())) return fail(saved);
        need(node, expect(TokenType::COLON, NodeType::Colon, ":"));
        if (!need(node, parseType())) return fail(saved);
        need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));
    } while (match(TokenType::IDENT));

    return node;
}

//8
std::shared_ptr<TreeNode> Parser::parseIdentifierList() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::IdentifierList);

    if (!need(node, terminal(TokenType::IDENT, NodeType::Ident))) return fail(saved);

    while (match(TokenType::COMMA)) {
        need(node, expect(TokenType::COMMA, NodeType::Comma, ","));
        need(node, expect(TokenType::IDENT, NodeType::Ident, "identifier"));
    }

    return node;
}

//9
std::shared_ptr<TreeNode> Parser::parseType() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Type);

    if (need(node, parseArrayType())) return node;
    restore(saved);
    if (need(node, parseRange())) return node;
    restore(saved);
    if (need(node, parseEnumerated())) return node;
    restore(saved);
    if (need(node, parseRecordType())) return node;
    restore(saved);
    if (need(node, terminal(TokenType::IDENT, NodeType::Ident))) return node;
    
    return fail(saved);

}

//10
std::shared_ptr<TreeNode> Parser::parseArrayType() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ArrayType);

    if (!need(node, terminal(TokenType::ARRAYSY, NodeType::ArraySy))) return fail(saved);
    need(node, expect(TokenType::LBRACK, NodeType::LBrack, "["));
    if (!(need(node, parseRange()) || need(node, terminal(TokenType::IDENT, NodeType::Ident)))) return fail(saved);
    need(node, expect(TokenType::RBRACK, NodeType::RBrack, "]"));
    need(node, expect(TokenType::OFSY, NodeType::OfSy, "of"));
    if (!need(node, parseType())) return fail(saved);

    return node;
}

//11
std::shared_ptr<TreeNode> Parser::parseRange() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Range);

    if (!need(node, parseConstant())) return fail(saved);
    if (!need(node, terminal(TokenType::PERIOD, NodeType::Period))) return fail(saved);
    need(node, expect(TokenType::PERIOD, NodeType::Period, "."));
    if (!need(node, parseConstant())) return fail(saved);

    return node;
}

//12
std::shared_ptr<TreeNode> Parser::parseEnumerated() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Enumerated);

    if(!need(node, terminal(TokenType::LPARENT, NodeType::LParent))) return fail(saved);
    need(node, expect(TokenType::IDENT, NodeType::Ident, "enumerated identifier"));
    
    while (match(TokenType::COMMA)) {
        need(node, expect(TokenType::COMMA, NodeType::Comma, ","));
        need(node, expect(TokenType::IDENT, NodeType::Ident, "enumerated identifier"));
    }

    need(node, expect(TokenType::RPARENT, NodeType::RParent, ")"));

    return node;
}

//13
std::shared_ptr<TreeNode> Parser::parseRecordType() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::RecordType);

    if (!need(node, terminal(TokenType::RECORDSY, NodeType::RecordSy))) return fail(saved);
    if (!need(node, parseFieldList())) return fail(saved);
    need(node, expect(TokenType::ENDSY, NodeType::EndSy, "end"));

    return node;
}

//14
std::shared_ptr<TreeNode> Parser::parseFieldList() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::FieldList);

    if (!need(node, parseFieldPart())) return fail(saved);

    while (match(TokenType::SEMICOLON)) {
        need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));
        if (!need(node, parseFieldPart())) return fail(saved);
    }
    return node;
}

//15
std::shared_ptr<TreeNode> Parser::parseFieldPart() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::FieldPart);

    if (!need(node, parseIdentifierList())) return fail(saved);
    need(node, expect(TokenType::COLON, NodeType::Colon, ":"));
    if (!need(node, parseType())) return fail(saved);

    return node;
}

//16
std::shared_ptr<TreeNode> Parser::parseSubprogramDeclaration() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::SubprogramDeclaration);

    if (need(node, parseProcedureDeclaration())) return node;
    restore(saved);
    if (need(node, parseFunctionDeclaration())) return node;

    return fail(saved);
}

//17
std::shared_ptr<TreeNode> Parser::parseProcedureDeclaration() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ProcedureDeclaration);

    if (!need(node, terminal(TokenType::PROCEDURESY, NodeType::ProcedureSy))) return fail(saved);
    need(node, expect(TokenType::IDENT, NodeType::Ident, "procedure name"));
    need(node, parseFormalParameterList());
    need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));
    if (!need(node, parseBlock())) return fail(saved);
    need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));

    return node;
}

//18
std::shared_ptr<TreeNode> Parser::parseFunctionDeclaration() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::FunctionDeclaration);

    if (!need(node, terminal(TokenType::FUNCTIONSY, NodeType::FunctionSy))) return fail(saved);
    need(node, expect(TokenType::IDENT, NodeType::Ident, "function name"));
    need(node, parseFormalParameterList());
    need(node, expect(TokenType::COLON, NodeType::Colon, ":"));
    need(node, expect(TokenType::IDENT, NodeType::Ident, "return type"));
    need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));
    if (!need(node, parseBlock())) return fail(saved);
    need(node, expect(TokenType::SEMICOLON, NodeType::Semicolon, ";"));

    return node;
}

//19
std::shared_ptr<TreeNode> Parser::parseBlock() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Block);

    if (!need(node, parseDeclarationPart())) return fail(saved);
    if (!need(node, parseCompoundStatement())) return fail(saved);

    return node;
}

//20
std::shared_ptr<TreeNode> Parser::parseFormalParameterList() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::FormalParameterList);

    if (!need(node, terminal(TokenType::LPARENT, NodeType::LParent))) return fail(saved);
    if (!need(node, parseParameterGroup())) return fail(saved);

    while (match(TokenType::SEMICOLON)) {
        if (!need(node, terminal(TokenType::SEMICOLON, NodeType::Semicolon))) return fail(saved);
        if (!need(node, parseParameterGroup())) return fail(saved);
    }

    need(node, expect(TokenType::RPARENT, NodeType::RParent, ")"));

    return node;
}

//21
std::shared_ptr<TreeNode> Parser::parseParameterGroup() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ParameterGroup);

    if (!need(node, parseIdentifierList())) return fail(saved);
    if (!need(node, terminal(TokenType::COLON, NodeType::Colon))) return fail(saved);
    if (!(need(node, terminal(TokenType::IDENT, NodeType::Ident)) || need(node, parseArrayType()))) return fail(saved);

    return node;
}

//22
std::shared_ptr<TreeNode> Parser::parseCompoundStatement() {
    size_t saved = save();
	auto node = std::make_shared<TreeNode>(NodeType::CompoundStatement);

    if(!need(node, terminal(TokenType::BEGINSY, NodeType::BeginSy))) return fail(saved);
    if(!need(node, parseStatementList())) return fail(saved);
    need(node, expect(TokenType::ENDSY, NodeType::EndSy, "end"));

    return node;
}

//23
std::shared_ptr<TreeNode> Parser::parseStatementList() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::StatementList);
 
    if (!need(node, parseStatement())) return fail(saved);
    while (match(TokenType::SEMICOLON)) {
        if (!need(node, terminal(TokenType::SEMICOLON, NodeType::Semicolon))) return fail(saved);
        if (!need(node, parseStatement())) return fail(saved);
    }

    return node;
}

//24
std::shared_ptr<TreeNode> Parser::parseStatement() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Statement);

    if (need(node, parseAssignmentStatement()) ||
        need(node, parseIfStatement()) ||
        need(node, parseCaseStatement()) ||
        need(node, parseWhileStatement()) ||
        need(node, parseRepeatStatement()) ||
        need(node, parseForStatement()) ||
        need(node, parseProcedureCall())) {
        return node;
    }

    restore(saved);

    return node;
}

//25
std::shared_ptr<TreeNode> Parser::parseAssignmentStatement() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::AssignmentStatement);

    if (!need(node, parseVariable())) return fail(saved);
    if (!need(node, terminal(TokenType::BECOMES, NodeType::Becomes))) return fail(saved);
    if (!need(node, parseExpression())) {
         throw std::runtime_error("Syntax Error: Expected an expression after ':='");
    }

    return node;
}

//26
std::shared_ptr<TreeNode> Parser::parseIfStatement() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::IfStatement);

    if (!need(node, terminal(TokenType::IFSY, NodeType::IfSy))) return fail(saved);
    if (!need(node, parseExpression())) return fail(saved);
    need(node, expect(TokenType::THENSY, NodeType::ThenSy, "then"));
    if (!need(node, parseStatement())) return fail(saved);

    if (match(TokenType::ELSESY)) {
        if (!need(node, terminal(TokenType::ELSESY, NodeType::ElseSy))) return fail(saved);
        if (!need(node, parseStatement())) return fail(saved);
    }

    return node;
}

//27
std::shared_ptr<TreeNode> Parser::parseCaseStatement() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::CaseStatement);

    if (!need(node, terminal(TokenType::CASESY, NodeType::CaseSy))) return fail(saved);
    if (!need(node, parseExpression())) return fail(saved);
    need(node, expect(TokenType::OFSY, NodeType::OfSy, "of"));
    if (!need(node, parseCaseBlock())) return fail(saved);
    need(node, expect(TokenType::ENDSY, NodeType::EndSy, "end"));

    return node;
}

//28
std::shared_ptr<TreeNode> Parser::parseCaseBlock() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::CaseBlock);

    if (!need(node, parseConstant())) return fail(saved);

    while (match(TokenType::COMMA)) {
        if (!need(node, terminal(TokenType::COMMA, NodeType::Comma))) return fail(saved);
        if (!need(node, parseConstant())) return fail(saved);
    }

    need(node, expect(TokenType::COLON, NodeType::Colon, ":"));
    if (!need(node, parseStatement())) return fail(saved);
    
    while (match(TokenType::SEMICOLON)) {
        if (!need(node, terminal(TokenType::SEMICOLON, NodeType::Semicolon))) return fail(saved);
        need(node, parseCaseBlock());
    }

    return node;
}

//29
std::shared_ptr<TreeNode> Parser::parseWhileStatement() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::WhileStatement);

    if (!need(node, terminal(TokenType::WHILESY, NodeType::WhileSy))) return fail(saved);
    if (!need(node, parseExpression())) return fail(saved);
    need(node, expect(TokenType::DOSY, NodeType::DoSy, "do"));
    if (!need(node, parseStatement())) return fail(saved);

    return node;
}

//30
std::shared_ptr<TreeNode> Parser::parseRepeatStatement() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::RepeatStatement);

    if (!need(node, terminal(TokenType::REPEATSY, NodeType::RepeatSy))) return fail(saved);
    if (!need(node, parseStatementList())) return fail(saved);
    need(node, expect(TokenType::UNTILSY, NodeType::UntilSy, "until"));
    if (!need(node, parseExpression())) return fail(saved);

    return node;
}

//31
std::shared_ptr<TreeNode> Parser::parseForStatement() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ForStatement);

    if (!need(node, terminal(TokenType::FORSY, NodeType::ForSy))) return fail(saved);
    need(node, expect(TokenType::IDENT, NodeType::Ident, "iterator variable"));
    need(node, expect(TokenType::BECOMES, NodeType::Becomes, ":="));
    if (!need(node, parseExpression())) return fail(saved);
    if (!(need(node, terminal(TokenType::TOSY, NodeType::ToSy)) || need(node, terminal(TokenType::DOWNTOSY, NodeType::DownToSy)))) {
        throw std::runtime_error("Syntax Error: Expected 'to' or 'downto' in for loop.");
    }
    if (!need(node, parseExpression())) return fail(saved);
    need(node, expect(TokenType::DOSY, NodeType::DoSy, "do"));
    if (!need(node, parseStatement())) return fail(saved);

    return node;
}

//32
std::shared_ptr<TreeNode> Parser::parseProcedureCall() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ProcedureCall);

    if (!need(node, terminal(TokenType::IDENT, NodeType::Ident))) return fail(saved);
    if (!need(node, terminal(TokenType::LPARENT, NodeType::LParent))) return fail(saved);
    need(node, parseParameterList());
    need(node, expect(TokenType::RPARENT, NodeType::RParent, ")"));

    return node;
}

//33
std::shared_ptr<TreeNode> Parser::parseParameterList() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ParameterList);

    if (!need(node, parseExpression())) return fail(saved);

    while (match(TokenType::COMMA)) {
        if (!need(node, terminal(TokenType::COMMA, NodeType::Comma))) return fail(saved);
        if (!need(node, parseExpression())) return fail(saved);
    }

    return node;
}

//34
std::shared_ptr<TreeNode> Parser::parseExpression() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Expression);

    if (!need(node, parseSimpleExpression())) return fail(saved);

    if (need(node, parseRelationalOperator())) {
        if(!(need(node, parseSimpleExpression()))) return fail(saved);
    }

    return node;
}

//35
std::shared_ptr<TreeNode> Parser::parseSimpleExpression() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::SimpleExpression);

    need(node, terminal(TokenType::PLUS, NodeType::Plus)) || 
    need(node, terminal(TokenType::MINUS, NodeType::Minus));

    if (!need(node, parseTerm())) return fail(saved);

    while (need(node, parseAdditiveOperator())) {
        if (!need(node, parseTerm())) return fail(saved);
    }

    return node;
}

//36
std::shared_ptr<TreeNode> Parser::parseTerm() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Term);

    if (!need(node, parseFactor())) return fail(saved);

    while (need(node, parseMultiplicativeOperator())) {
        if (!need(node, parseFactor())) return fail(saved);
    }

    return node;
}

//37
std::shared_ptr<TreeNode> Parser::parseFactor() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Factor);
    
    // cek procedure/function call
    size_t callSaved = save();
    if (need(node, parseProcedureCall())) return node;
    restore(callSaved);

    // cek variable
    if (need(node, parseVariable())) return node;
    restore(saved);

    // cek const
    if (need(node, terminal(TokenType::INTCON, NodeType::IntCon)) ||
        need(node, terminal(TokenType::REALCON, NodeType::RealCon)) ||
        need(node, terminal(TokenType::CHARCON, NodeType::CharCon)) ||
        need(node, terminal(TokenType::STRINGCON, NodeType::String))) {
        return node;
    }
    restore(saved);

    // cek (lparent + expression + rparent)
    if (match(TokenType::LPARENT)) {
        if (!need(node, terminal(TokenType::LPARENT, NodeType::LParent))) return fail(saved);
        if (!need(node, parseExpression())) return fail(saved);
        need(node, expect(TokenType::RPARENT, NodeType::RParent, ")"));
        return node;
    }
    restore(saved);

    // cek (notsy + factor)
    if (match(TokenType::NOTSY)) {
        if (!need(node, terminal(TokenType::NOTSY, NodeType::NotSy))) return fail(saved);
        if (!need(node, parseFactor())) return fail(saved);
        return node;
    }    
    
    return fail(saved);
}

//38
std::shared_ptr<TreeNode> Parser::parseRelationalOperator() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::RelationalOperator);

    if (need(node, terminal(TokenType::EQL, NodeType::Eql)) ||
    need(node, terminal(TokenType::NEQ, NodeType::Neq)) ||
    need(node, terminal(TokenType::GTR, NodeType::Gtr)) ||
    need(node, terminal(TokenType::GEQ, NodeType::Geq)) ||
    need(node, terminal(TokenType::LSS, NodeType::Lss)) ||
    need(node, terminal(TokenType::LEQ, NodeType::Leq))) return node;

    return fail(saved);
}

//39
std::shared_ptr<TreeNode> Parser::parseAdditiveOperator() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::AdditiveOperator);

    if (need(node, terminal(TokenType::PLUS, NodeType::Plus)) ||
    need(node, terminal(TokenType::MINUS, NodeType::Minus)) ||
    need(node, terminal(TokenType::ORSY, NodeType::OrSy))) return node;

    return fail(saved);
}

//40
std::shared_ptr<TreeNode> Parser::parseMultiplicativeOperator() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::MultiplicativeOperator);

    if (need(node, terminal(TokenType::TIMES, NodeType::Times)) ||
    need(node, terminal(TokenType::RDIV, NodeType::RDiv)) ||
    need(node, terminal(TokenType::IDIV, NodeType::IDiv)) ||
    need(node, terminal(TokenType::IMOD, NodeType::IMod)) ||
    need(node, terminal(TokenType::ANDSY, NodeType::AndSy))) return node;

    return fail(saved);
}

// Revisi
std::shared_ptr<TreeNode> Parser::parseVariable() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Variable);
    if(!need(node, terminal(TokenType::IDENT, NodeType::Ident))) return fail(saved);
    while (need(node, parseComponentVariable())) {}
    return node;
}

std::shared_ptr<TreeNode> Parser::parseIndexList() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::IndexList);
    
    if (!(need(node, terminal(TokenType::INTCON, NodeType::IntCon)) ||
          need(node, terminal(TokenType::CHARCON, NodeType::CharCon)) ||
          need(node, terminal(TokenType::IDENT, NodeType::Ident)))) return fail(saved);

    while (match(TokenType::COMMA)) {
        need(node, expect(TokenType::COMMA, NodeType::Comma, ","));

        if (!(need(node, terminal(TokenType::INTCON, NodeType::IntCon)) ||
              need(node, terminal(TokenType::CHARCON, NodeType::CharCon)) ||
              need(node, terminal(TokenType::IDENT, NodeType::Ident)))) return fail(saved);
    }

    return node;
}

std::shared_ptr<TreeNode> Parser::parseComponentVariable() {
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ComponentVariable);

    if (match(TokenType::LBRACK)) {
        if (!need(node, terminal(TokenType::LBRACK, NodeType::LBrack))) return fail(saved);
        if (!need(node, parseIndexList())) return fail(saved);
        need(node, expect(TokenType::RBRACK, NodeType::RBrack, "]"));
        return node;
    }
    else if (match(TokenType::PERIOD)) {
        if (!need(node, terminal(TokenType::PERIOD, NodeType::Period))) return fail(saved);
        need(node, expect(TokenType::IDENT, NodeType::Ident, "record field identifier"));
        return node;
    }

    return fail(saved);
}

const Token& Parser::peek(int offset) const {
	return tokens[pos + offset];
}

const Token& Parser::advance() {
	return tokens[pos++];
}


std::string Parser::printNode(const std::shared_ptr<TreeNode> &node){
    if (node == nullptr){
        return "";
    }

    std::string label = node->typeString();
    if (!node->value.empty()){
        label += "(" + node->value + ")";
    }
    return label;
}

void Parser::printParseTree(const std::shared_ptr<TreeNode> &node, const std::string &prefix = "", bool last = true, bool root = true){
    if (node == nullptr)
    {
        return;
    }

    if (root)
    {
        std::cout << printNode(node) << '\n';
    }
    else
    {
        std::cout << prefix << (last ? "└── " : "├── ") << printNode(node) << '\n';
    }

    std::string childPrefix = prefix;
    if (!root)
    {
        childPrefix += last ? "    " : "│   ";
    }

    for (size_t i = 0; i < node->children.size(); ++i)
    {
        printParseTree(node->children[i], childPrefix, i + 1 == node->children.size(), false);
    }
}