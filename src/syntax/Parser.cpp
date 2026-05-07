
#include "Parser.hpp"
#include "iostream"

Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens), pos(0) {}

bool Parser::match(TokenType type) const
{
    return pos < tokens.size() && tokens[pos].type == type;
}

size_t Parser::save() const
{
    return pos;
}

void Parser::restore(size_t saved)
{
    pos = saved;
}

std::shared_ptr<TreeNode> Parser::fail(size_t saved)
{
    restore(saved);
    return nullptr;
}

std::shared_ptr<TreeNode> Parser::terminal(TokenType tokenType, NodeType nodeType)
{
    if (!match(tokenType)) {
        return nullptr;
    }

    auto value = tokens[pos].value;
    pos++;

    return std::make_shared<TreeNode>(nodeType, value);
}

std::shared_ptr<TreeNode> Parser::expect(TokenType tokenType, NodeType nodeType, std::string expectedStr)
{
    if (match(tokenType)) {
        return terminal(tokenType, nodeType);
    }
    const Token &currentToken = peek();
    std::string detailMsg = "Diharapkan " + expectedStr + " tetapi menemukan '" + currentToken.value + "'";
    SyntaxError err(detailMsg, currentToken.line, currentToken.column);
    syntaxErrors.push_back(err.what());
    std::shared_ptr<TreeNode> errorNode = std::make_shared<TreeNode>(nodeType, "<Missing " + expectedStr + ">", true);
    return errorNode;
}

/**
 * Untuk melakukan Panic Mode Recovery
 *
 * So caranya jika ditemukan token yang salah maka
 * akan memncari toekn samapai menemukan token yang aman untuk melakukan
 * parsing berikutnya
 * Pijakan yang aman
 * 1. Berakhir dari ;
 * 2. Awalan dari statement baru if, while, var, begin
 */
void Parser::synchronize()
{
    advance();
    while (pos < tokens.size()) {
        // Jika token sebelumnya ; sudah melewati error
        if (peek(-1).type == TokenType::SEMICOLON) {
            return;
        }
        // jika token adalah awal dari statement atau blok baru,
        // berhenti membuang token
        switch (peek().type) {
            case TokenType::VARSY:
            case TokenType::CONSTSY:
            case TokenType::TYPESY:
            case TokenType::PROCEDURESY:
            case TokenType::FUNCTIONSY:
            case TokenType::BEGINSY:
            case TokenType::IFSY:
            case TokenType::WHILESY:
            case TokenType::REPEATSY:
            case TokenType::FORSY:
            case TokenType::CASESY:
                return;
            default:
                break;
        }
        advance();
    }
}

// this function checks wether the child exist or not
bool Parser::need(const std::shared_ptr<TreeNode> &parent, const std::shared_ptr<TreeNode> &child)
{
    if (child == nullptr) {
        return false;
    }

    parent->addChild(child);
    return true;
}

// 1 PROGRAM → PROGRAM-HEADER + DECLARATION-PART + COMPOUND-STATEMENT + period
std::shared_ptr<TreeNode> Parser::parseProgram()
{
    auto node = std::make_shared<TreeNode>(NodeType::Program);

    auto header = parseProgramHeader();
    if (header) node->addChild(header);

    auto declPart = parseDeclarationPart();
    if (declPart) node->addChild(declPart);

    auto compStmt = parseCompoundStatement();
    if (compStmt) node->addChild(compStmt);

    auto period = expect(TokenType::PERIOD, NodeType::Period, ".");
    need(node, period);

    return node;
}

// 2 PROGRAM-HEADER → programsy + ident + semicolon 
std::shared_ptr<TreeNode> Parser::parseProgramHeader()
{
    auto node = std::make_shared<TreeNode>(NodeType::ProgramHeader);

    need(node, expect(TokenType::PROGRAMSY, NodeType::ProgramSy, "'program'"));
    need(node, expect(TokenType::IDENT, NodeType::Ident, "identifier program"));
    auto semi = expect(TokenType::SEMICOLON, NodeType::Semicolon, "';'");
    need(node, semi);

    if (semi->isError) {
        synchronize();
    }

    return node;
}

// 3 DECLARATION-PART → (CONST-DECLARATION)* + (TYPE-DECLARATION)* + (VAR-DECLARATION)* + (SUBPROGRAM-DECLARATION)*
std::shared_ptr<TreeNode> Parser::parseDeclarationPart()
{
    auto node = std::make_shared<TreeNode>(NodeType::DeclarationPart);

    // a* + b* + c* + d*
    while (need(node, parseConstDeclaration()));
    while (need(node, parseTypeDeclaration()));
    while (need(node, parseVarDeclaration()));
    while (need(node, parseSubprogramDeclaration()));
    return node;
}

// 4 CONST-DECLARATION → constsy + (ident + eql + CONSTANT + semicolon)
std::shared_ptr<TreeNode> Parser::parseConstDeclaration()
{
    if (peek().type != TokenType::CONSTSY) return nullptr;
    auto node = std::make_shared<TreeNode>(NodeType::ConstDeclaration);
    need(node, terminal(TokenType::CONSTSY, NodeType::ConstSy));
    do {
        auto idNode = expect(TokenType::IDENT, NodeType::Ident, "identifier konstanta");
        need(node, idNode);
        if (idNode->isError) synchronize();

        auto eqlNode = expect(TokenType::EQL, NodeType::Eql, "'=='");
        need(node, eqlNode);

        auto constVal = parseConstant();
        if (constVal) {
            need(node, constVal);
        }
        else {
            SyntaxError err("Diharapkan nilai konstanta", peek().line, peek().column);
            syntaxErrors.push_back(err.what());

            auto errNode = std::make_shared<TreeNode>(NodeType::Constant, "<Missing Constant>", true);
            node->addChild(errNode);
            synchronize();
        }

        auto semi = expect(TokenType::SEMICOLON, NodeType::Semicolon, "';'");
        need(node, semi);

        // PENTING: Jika titik koma error, kita sync untuk mencoba memulihkan
        if (semi->isError) synchronize();

    } while (peek().type == TokenType::IDENT);

    return node;
}

// 5 CONSTANT → charcon | string | [(plus | minus )? + (ident | intcon | realcon)]
std::shared_ptr<TreeNode> Parser::parseConstant()
{
    auto node = std::make_shared<TreeNode>(NodeType::Constant);

    // Coba opsi yang valid satu per satu
    if (auto c = terminal(TokenType::CHARCON, NodeType::CharCon)) {
        node->addChild(c);
        return node;
    }
    if (auto s = terminal(TokenType::STRINGCON, NodeType::String)) {
        node->addChild(s);
        return node;
    }

    // Jika ada plus/minus (opsional)
    if (auto p = terminal(TokenType::PLUS, NodeType::Plus)) node->addChild(p);
    else if (auto m = terminal(TokenType::MINUS, NodeType::Minus)) node->addChild(m);

    // Jika sudah ada plus/minus, atau kosongan, cari nilainya
    if (auto id = terminal(TokenType::IDENT, NodeType::Ident)) {
        node->addChild(id);
        return node;
    }
    if (auto ic = terminal(TokenType::INTCON, NodeType::IntCon)) {
        node->addChild(ic);
        return node;
    }
    if (auto rc = terminal(TokenType::REALCON, NodeType::RealCon)) {
        node->addChild(rc);
        return node;
    }

    // Jika kita sudah mengonsumsi plus/minus TAPI tidak menemukan angka/ident setelahnya
    if (!node->children.empty()) {
        SyntaxError err("Diharapkan angka atau identifier setelah tanda +/-", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Number>", true));
        return node;
    }
    // Jika sama sekali bukan konstanta, kembalikan nullptr
    return nullptr;
}

// 6 TYPE-DECLARATION → typesy + (ident + eql + TYPE + semicolon)+ TYPE-DECLARATION
std::shared_ptr<TreeNode> Parser::parseTypeDeclaration()
{
    if (peek().type != TokenType::TYPESY) return nullptr;

    auto node = std::make_shared<TreeNode>(NodeType::TypeDeclaration);
    need(node, terminal(TokenType::TYPESY, NodeType::TypeSy));

    do {
        auto idNode = expect(TokenType::IDENT, NodeType::Ident, "identifier tipe");
        need(node, idNode);
        if (idNode->isError) synchronize();
        auto eqlNode = expect(TokenType::EQL, NodeType::Eql, "'=='");
        need(node, eqlNode);
        auto typeNode = parseType();
        if (typeNode) {
            need(node, typeNode);
        }
        else {
            SyntaxError err("Diharapkan definisi tipe", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Type, "<Missing Type>", true));
            synchronize();
        }
        auto semi = expect(TokenType::SEMICOLON, NodeType::Semicolon, "';'");
        need(node, semi);
        if (semi->isError) synchronize();
    } while (peek().type == TokenType::IDENT);

    return node;
}

// 7 VAR-DECLARATION →  varsy + (IDENTIFIER-LIST + colon + TYPE + semicolon) + VAR-DECLARATION
std::shared_ptr<TreeNode> Parser::parseVarDeclaration()
{
    if (peek().type != TokenType::VARSY) return nullptr;

    auto node = std::make_shared<TreeNode>(NodeType::VarDeclaration);
    need(node, terminal(TokenType::VARSY, NodeType::VarSy));

    do {
        auto idList = parseIdentifierList();
        if (idList) need(node, idList);

        auto colonNode = expect(TokenType::COLON, NodeType::Colon, "':'");
        need(node, colonNode);

        auto typeNode = parseType();
        if (typeNode) {
            need(node, typeNode);
        }
        else {
            SyntaxError err("Diharapkan tipe data variabel", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Type, "<Missing Type>", true));
            synchronize();
        }

        auto semi = expect(TokenType::SEMICOLON, NodeType::Semicolon, "';'");
        need(node, semi);
        if (semi->isError) synchronize();

    } while (peek().type == TokenType::IDENT);

    return node;
}

// 8 IDENTIFIER-LIST → ident (comma + ident)*
std::shared_ptr<TreeNode> Parser::parseIdentifierList()
{
    auto node = std::make_shared<TreeNode>(NodeType::IdentifierList);

    auto firstId = expect(TokenType::IDENT, NodeType::Ident, "identifier");
    need(node, firstId);

    if (firstId->isError) return node;

    while (peek().type == TokenType::COMMA) {
        need(node, expect(TokenType::COMMA, NodeType::Comma, ","));
        need(node, expect(TokenType::IDENT, NodeType::Ident, "identifier"));
    }

    return node;
}

// 9 TYPE → ident | ARRAY-TYPE | RANGE |  ENUMERATED | RECORD-TYPE
std::shared_ptr<TreeNode> Parser::parseType()
{
    auto node = std::make_shared<TreeNode>(NodeType::Type);

    if (peek().type == TokenType::ARRAYSY) {
        need(node, parseArrayType());
        return node;
    }

    auto rangeNode = parseRange();
    if (rangeNode) {
        need(node, rangeNode);
        return node;
    }

    auto enumNode = parseEnumerated();
    if (enumNode) {
        need(node, enumNode);
        return node;
    }

    auto recordNode = parseRecordType();
    if (recordNode) {
        need(node, recordNode);
        return node;
    }

    // Jika bukan array, range, enum, atau record, maka HARUS identifier
    // (misal tipe data primitif kustom atau bawaan spt 'integer', 'boolean')
    if (peek().type == TokenType::IDENT) {
        need(node, terminal(TokenType::IDENT, NodeType::Ident));
        return node;
    }

    // Jika sampai di sini, berarti bukan tipe data valid sama sekali
    return nullptr;
}

// 10 ARRAY-TYPE → arraysy + lbrack + (RANGE | ident) + rbrack + ofsy + TYPE
std::shared_ptr<TreeNode> Parser::parseArrayType()
{
    if (peek().type != TokenType::ARRAYSY) return nullptr;

    auto node = std::make_shared<TreeNode>(NodeType::ArrayType);

    need(node, terminal(TokenType::ARRAYSY, NodeType::ArraySy));

    auto lBrack = expect(TokenType::LBRACK, NodeType::LBrack, "'['");
    need(node, lBrack);
    if (lBrack->isError) synchronize();

    auto rangeNode = parseRange();
    if (rangeNode) {
        need(node, rangeNode);
    }
    else {
        auto idNode = expect(TokenType::IDENT, NodeType::Ident, "range atau identifier");
        need(node, idNode);
    }

    auto rBrack = expect(TokenType::RBRACK, NodeType::RBrack, "']'");
    need(node, rBrack);
    if (rBrack->isError) synchronize();

    auto ofNode = expect(TokenType::OFSY, NodeType::OfSy, "'of'");
    need(node, ofNode);

    auto typeNode = parseType();
    if (typeNode) {
        need(node, typeNode);
    }
    else {
        SyntaxError err("Diharapkan tipe data untuk elemen array", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Type, "<Missing Array Type>", true));
    }

    return node;
}

// 11 RANGE → CONSTANT + period + period + CONSTANT
std::shared_ptr<TreeNode> Parser::parseRange()
{   
    size_t saved = save();
    auto const1 = parseConstant();
    if (!const1 || peek().type != TokenType::PERIOD) {
        restore(saved);
        return nullptr;
    }
    auto node = std::make_shared<TreeNode>(NodeType::Range);
    need(node, const1);

    auto periodNode1 = expect(TokenType::PERIOD, NodeType::Period, ".");
    need(node, periodNode1);
    if (periodNode1->isError) synchronize();

    auto periodNode2 = expect(TokenType::PERIOD, NodeType::Period, ".");
    need(node, periodNode2);
    if (periodNode2->isError) synchronize();

    auto const2 = parseConstant();
    if (const2) {
        need(node, const2);
    }
    else {
        SyntaxError err("Diharapkan Konstanta (batas) di baris ",peek().line,peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Constant>", true));
        synchronize();
    }
    return node;
}

// 12 ENUMERATED → lparent + ident + (comma + ident)* + rparent
std::shared_ptr<TreeNode> Parser::parseEnumerated()
{
    if (peek().type != TokenType::LPARENT){
        return nullptr;
    }
    
    auto node = std::make_shared<TreeNode>(NodeType::Enumerated);
    auto lpar = expect(TokenType::LPARENT, NodeType::LParent, "'('");
    need(node, lpar);

    auto ident = expect(TokenType::IDENT, NodeType::Ident,"enumerated identifier" );
    need(node, ident);
    if (ident->isError) synchronize();

    while (peek().type == TokenType::COMMA) {
        need(node, terminal(TokenType::COMMA, NodeType::Comma));

        auto nextIden = expect(TokenType::IDENT, NodeType::Ident, "enumerated identifier setelah koma");
        need(node, nextIden);
        if (nextIden->isError) synchronize();
    }

    auto rpar = expect(TokenType::RPARENT, NodeType::RParent, "')'");
    need(node, rpar);
    if (rpar->isError) synchronize();
    return node;
}

// 13 RECORD-TYPE → recordsy + FIELD-LIST + endsy
std::shared_ptr<TreeNode> Parser::parseRecordType()
{
    if (peek().type != TokenType::RECORDSY){
        return nullptr;
    }
    
    auto node = std::make_shared<TreeNode>(NodeType::RecordType);

    auto recordsy = expect(TokenType::RECORDSY, NodeType::RecordSy, "record type");
    need(node, recordsy);

    if (recordsy->isError) synchronize();
    auto fieldList= parseFieldList();
    if (fieldList){
        need(node, fieldList);
    }else{
        SyntaxError err("Diharapkan daftar field", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Field-List>", true));
        synchronize();
    }
    
    auto endsy = expect(TokenType::ENDSY, NodeType::EndSy, "end");
    need(node, endsy);
    if (endsy->isError){
        synchronize();
    }
    return node;
}

// 14 FIELD-LIST → FIELD-PART + (semicolon + FIELD-PART)*
std::shared_ptr<TreeNode> Parser::parseFieldList()
{
    auto node = std::make_shared<TreeNode>(NodeType::FieldList);

    auto firstField = parseFieldPart();
    if (firstField)
    {
        need(node, firstField);
    }else{
        return nullptr;
    }

    while (peek().type == (TokenType::SEMICOLON)) {
        need(node, terminal(TokenType::SEMICOLON, NodeType::Semicolon));

        auto nextField = parseFieldPart();
        if (nextField){
            need(node, nextField);
        }else{
            SyntaxError err("Diharapkan field-part setelah ';'", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Field-Part>", true));
            synchronize();
        }
    }
    return node;
}

// 15 FIELD-PART -> IDENTIFIER-LIST+  colon + TYPE
std::shared_ptr<TreeNode> Parser::parseFieldPart()
{   
    auto node = std::make_shared<TreeNode>(NodeType::FieldPart);
    auto idenList = parseIdentifierList();
    if (idenList){
        need(node, idenList);
    }else{
        SyntaxError err("Diharapkan daftar Identifier", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing IdentifierList>", true));
        synchronize();
    }
    
    auto colon = expect(TokenType::COLON, NodeType::Colon, ":");
    need(node, colon);
    if (colon->isError){
        synchronize();
    }
    
    auto type = parseType();
    if (type)
    {
        need(node, type);
    }else{
        SyntaxError err("Diharapkan Type Data", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Type>", true));
        synchronize();
    }
    return node;
}

// 16 SUBPROGRAM-DECLARATION -> PROCEDURE-DECLARATION | FUNCTION-DECLARATION

std::shared_ptr<TreeNode> Parser::parseSubprogramDeclaration()
{
    if (peek().type == TokenType::PROCEDURESY) {
        return parseProcedureDeclaration(); 
    } else if (peek().type == TokenType::FUNCTIONSY) {
        return parseFunctionDeclaration();  
    }
    
    return nullptr;
}

// 17 PROCEDURE-DECLARATION -> proceduresy + ident + (FORMAL-PARAMETER-LIST)? + semicolon + BLOCK + semicolon
std::shared_ptr<TreeNode> Parser::parseProcedureDeclaration()
{
    auto node = std::make_shared<TreeNode>(NodeType::ProcedureDeclaration);
    need(node, terminal(TokenType::PROCEDURESY, NodeType::ProcedureSy));

    auto idNode = expect(TokenType::IDENT, NodeType::Ident, "nama procedure");
    need(node, idNode);
    if (idNode->isError) synchronize();

    auto params = parseFormalParameterList();
    if (params) need(node, params);

    auto semi1 = expect(TokenType::SEMICOLON, NodeType::Semicolon, "';'");
    need(node, semi1);
    if (semi1->isError) synchronize();

    auto blockNode = parseBlock();
    if (blockNode) {
        need(node, blockNode);
    } else {
        SyntaxError err("Diharapkan blok kode (begin...end) pada procedure", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Block>", true));
        synchronize();
    }
    
    auto semi2 = expect(TokenType::SEMICOLON, NodeType::Semicolon, "';' di akhir procedure");
    need(node, semi2);
    if (semi2->isError) synchronize();

    return node;
}

// 18 FUNCTION-DECLARATION -> functionsy + ident + (FORMAL-PARAMETER-LIST)? + colon + ident + semicolon+ BLOCK + semicolon
std::shared_ptr<TreeNode> Parser::parseFunctionDeclaration()
{
    auto node = std::make_shared<TreeNode>(NodeType::FunctionDeclaration);

    need(node, terminal(TokenType::FUNCTIONSY, NodeType::FunctionSy));

    auto idNode = expect(TokenType::IDENT, NodeType::Ident, "nama function");
    need(node, idNode);
    if (idNode->isError) synchronize();

    // Opsional
    auto params = parseFormalParameterList();
    if (params) need(node, params);

    auto colonNode = expect(TokenType::COLON, NodeType::Colon, "':'");
    need(node, colonNode);
    if (colonNode->isError) synchronize();

    auto retType = expect(TokenType::IDENT, NodeType::Ident, "tipe kembalian function");
    need(node, retType);
    if (retType->isError) synchronize();

    auto semi1 = expect(TokenType::SEMICOLON, NodeType::Semicolon, "';'");
    need(node, semi1);
    if (semi1->isError) synchronize();

    auto blockNode = parseBlock();
    if (blockNode) need(node, blockNode);

    auto semi2 = expect(TokenType::SEMICOLON, NodeType::Semicolon, "';' di akhir function");
    need(node, semi2);
    if (semi2->isError) synchronize();

    return node;
}

// 19 BLOCK -> DECLARATION-PART + COMPOUND-STATEMENT
std::shared_ptr<TreeNode> Parser::parseBlock()
{
    auto node = std::make_shared<TreeNode>(NodeType::Block);

    auto declPart = parseDeclarationPart();
    if (declPart) need(node, declPart); 

    auto compStmt = parseCompoundStatement();
    if (compStmt) {
        need(node, compStmt);
    } else {
        SyntaxError err("Diharapkan blok 'begin ... end' (Compound Statement)", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Compound-Statement>", true));
        if (peek().type != TokenType::SEMICOLON) {
            synchronize(); 
        }
    }

    return node;
}

// 20 FORMAL-PARAMETER-LIST -> lparent + PARAMETER-GROUP + (semicolon + PARAMETER-GROUP)* + rparent
std::shared_ptr<TreeNode> Parser::parseFormalParameterList()
{
    if (peek().type != TokenType::LPARENT) {
        return nullptr; 
    }

    auto node = std::make_shared<TreeNode>(NodeType::FormalParameterList);

    auto lpar = expect(TokenType::LPARENT, NodeType::LParent, "'('");
    need(node, lpar);

    auto paramGroup = parseParameterGroup();
    if (paramGroup) {
        need(node, paramGroup);
    } else {
        SyntaxError err("Diharapkan definisi parameter", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing ParameterGroup>", true));
        synchronize();
    }

    while (peek().type == TokenType::SEMICOLON) {
        need(node, terminal(TokenType::SEMICOLON, NodeType::Semicolon)); // Konsumsi ';'
        
        auto nextParamGroup = parseParameterGroup();
        if (nextParamGroup) {
            need(node, nextParamGroup);
        } else {
            SyntaxError err("Diharapkan parameter setelah ';'", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing ParameterGroup>", true));
            synchronize();
        }
    }

    auto rpar = expect(TokenType::RPARENT, NodeType::RParent, "')'");
    need(node, rpar);
    if (rpar->isError) synchronize();

    return node;
}

// 21 PARAMETER-GROUP -> IDENTIFIER-LIST + colon + (ident | ARRAY-TYPE)
std::shared_ptr<TreeNode> Parser::parseParameterGroup()
{
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::ParameterGroup);

    if (!need(node, parseIdentifierList())) return fail(saved);
    if (!need(node, terminal(TokenType::COLON, NodeType::Colon))) return fail(saved);
    if (!(need(node, terminal(TokenType::IDENT, NodeType::Ident)) || need(node, parseArrayType()))) return fail(saved);

    return node;
}

// 22 COMPOUND-STATEMENT -> beginsy + STATEMENT-LIST + endsy
std::shared_ptr<TreeNode> Parser::parseCompoundStatement()
{
    if (peek().type != TokenType::BEGINSY) {
        return nullptr; 
    }
    auto node = std::make_shared<TreeNode>(NodeType::CompoundStatement);
    need(node, terminal(TokenType::BEGINSY, NodeType::BeginSy));
    auto stmtList = parseStatementList();
    if (stmtList) {
        need(node, stmtList);
    } else {
        SyntaxError err("Diharapkan statement di dalam blok 'begin'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Statement-List>", true));
        synchronize();
    }
    auto endSy = expect(TokenType::ENDSY, NodeType::EndSy, "'end'");
    need(node, endSy);
    if (endSy->isError) {
        synchronize();
    }
    return node;
}

// 23 STATEMENT-LIST -> STATEMENT (semicolon + STATEMENT)*
std::shared_ptr<TreeNode> Parser::parseStatementList()
{
    auto node = std::make_shared<TreeNode>(NodeType::StatementList);
    auto firstStatement = parseStatement();
    if (firstStatement) {
        need(node, firstStatement);
    } else {
        return nullptr;
    }

    while (peek().type == TokenType::SEMICOLON) {
        need(node, terminal(TokenType::SEMICOLON, NodeType::Semicolon)); // Aman memakan token di sini
        
        auto nextStatement = parseStatement();
        if (nextStatement) {
            need(node, nextStatement);
        } else {
            SyntaxError err("Diharapkan statement setelah ';'", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Statement>", true));
            synchronize();
        }
    }
    return node;
}

// 24 STATEMENT -> (ASSIGNMENT-STATEMENT | IF-STATEMENT | CASE-STATEMENT | WHILE-STATEMENT | REPEAT-STATEMENT | FOR-STATEMENT )? | PROCEDURE/FUNCTION-CALL
std::shared_ptr<TreeNode> Parser::parseStatement()
{
    auto node = std::make_shared<TreeNode>(NodeType::Statement);
    std::shared_ptr<TreeNode> child = nullptr;

    TokenType type = peek().type;

    if      (type == TokenType::IFSY)     child = parseIfStatement();
    else if (type == TokenType::CASESY)   child = parseCaseStatement();
    else if (type == TokenType::WHILESY)  child = parseWhileStatement();
    else if (type == TokenType::REPEATSY) child = parseRepeatStatement();
    else if (type == TokenType::FORSY)    child = parseForStatement();
    else if (type == TokenType::IDENT) {
        size_t saved = save();
        auto varNode = parseVariable(); 
        
        if (peek().type == TokenType::BECOMES) { 
            restore(saved); 
            child = parseAssignmentStatement();
        } else { 
            restore(saved); 
            child = parseProcedureCall();
        }
    }

    if (child) {
        need(node, child);
        return node;
    }

    return node; 
}

// 25 ASSIGNMENT-STATEMENT -> ident + VARIABLE +  becomes + EXPRESSION
std::shared_ptr<TreeNode> Parser::parseAssignmentStatement()
{
    if (peek().type != TokenType::IDENT){
        return nullptr;
    }
    auto node = std::make_shared<TreeNode>(NodeType::AssignmentStatement);

    auto var = parseVariable();
    if (var){
        need(node, var);
    }else{
        SyntaxError err("Diharapkan Variable untuk diassign", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Variable>", true));
        synchronize();
    }
    

    auto becomesy = expect(TokenType::BECOMES, NodeType::Becomes, ":=");
    need(node, becomesy);
    if (becomesy->isError){
        synchronize();
    }
    auto expression = parseExpression();
    if (expression){
        need(node, expression);
    }else{
        SyntaxError err("Diharapkan ada ekspresi untuk diassign", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Expression>", true));
        synchronize();
    }

    return node;
}

// 26 IF-STATEMENT -> ifsy + EXPRESSION + thensy + STATEMENT + (elsy + STATEMENT)?
std::shared_ptr<TreeNode> Parser::parseIfStatement()
{
    if (peek().type != TokenType::IFSY){
        return nullptr;
    }
    auto node = std::make_shared<TreeNode>(NodeType::IfStatement);
    auto ifsy = terminal(TokenType::IFSY, NodeType::IfSy);    
    need(node,ifsy);

    auto expression = parseExpression();
    if (expression){
        need(node, expression);
    }else{
        SyntaxError err("Diharapkan ekspresi kondisi setelah 'if'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Expression>", true));
        synchronize();
    }
    auto thensy = expect(TokenType::THENSY, NodeType::ThenSy, "'then'");
    need(node, thensy);
    if (thensy->isError){
        synchronize();
    }
    auto statement = parseStatement();
    if (statement){
        need(node,statement);
    }else{
        SyntaxError err("Diharapkan statement setelah 'then'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Statement>", true));
        synchronize();
    }

    if (match(TokenType::ELSESY)) {
        auto elsesy = terminal(TokenType::ELSESY, NodeType::ElseSy);
        need(node, elsesy);

        auto elseStatement = parseStatement();
        if (elseStatement) {
            need(node, elseStatement);
        } else {
            SyntaxError err("Diharapkan statement setelah 'else'", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Else-Statement>", true));
            synchronize();
        }
    }

    return node;
}

// 27 CASE-STATEMENT -> casesy + EXPRESSION + ofsy + CASE-BLOCK + endsy
std::shared_ptr<TreeNode> Parser::parseCaseStatement()
{
    if (peek().type != TokenType::CASESY){
        return nullptr;
    }
    auto node = std::make_shared<TreeNode>(NodeType::CaseStatement);

    auto casesy = terminal(TokenType::CASESY, NodeType::CaseSy);
    need(node, casesy);

    auto expression = parseExpression();
    if (expression){
        need(node, expression);
    }else{
        SyntaxError err("Diharapkan ada ekspresi setelah 'case'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Expression>", true));
        synchronize();
    }
    auto ofsy = expect(TokenType::OFSY, NodeType::OfSy, "of");
    need(node,ofsy);
    if (ofsy->isError){
        synchronize();
    }
    
    auto caseBlock = parseCaseBlock();
    if (caseBlock){
        need(node, caseBlock);
    }else{
         SyntaxError err("Diharapkan ada Case Block seteleh 'of'", peek().line, peek().column);
         syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Case Block>", true));
        synchronize();
    }

    auto endsy = expect(TokenType::ENDSY, NodeType::EndSy, "end");
    need(node, endsy);
    if (endsy->isError){
        synchronize();
    }
    

    return node;
}

// 28 CASE-BLOCK -> CONSTANT + (comma + CONSTANT)* + colon + STATEMENT +  (semicolon + CASE-BLOCK?)*
std::shared_ptr<TreeNode> Parser::parseCaseBlock()
{ 
    auto firstConst = parseConstant();
    if (!firstConst) return nullptr; 

    auto node = std::make_shared<TreeNode>(NodeType::CaseBlock);
    need(node, firstConst);

    while (peek().type == TokenType::COMMA) {
        need(node, terminal(TokenType::COMMA, NodeType::Comma));
        
        auto nextConst = parseConstant();
        if (nextConst) {
            need(node, nextConst);
        } else {
            SyntaxError err("Diharapkan konstanta setelah ','", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Constant>", true));
            synchronize();
        }
    }

    auto colon = expect(TokenType::COLON, NodeType::Colon, "':'");
    need(node, colon);
    if (colon->isError) synchronize();

    auto stmt = parseStatement();
    if (stmt) {
        need(node, stmt);
    } else {
        SyntaxError err("Diharapkan statement setelah ':'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Statement>", true));
        synchronize();
    }

    while (peek().type == TokenType::SEMICOLON) {
        need(node, terminal(TokenType::SEMICOLON, NodeType::Semicolon)); 
        
        auto nextCase = parseCaseBlock();
        if (nextCase) {
            need(node, nextCase);
        } 
    }

    return node;
}

// 29 WHILE-STATEMENT -> whilesy + EXPRESSION + dosy + STATEMENT 
std::shared_ptr<TreeNode> Parser::parseWhileStatement()
{
    if (peek().type != TokenType::WHILESY) return nullptr;

    auto node = std::make_shared<TreeNode>(NodeType::WhileStatement);
    need(node, terminal(TokenType::WHILESY, NodeType::WhileSy));

    auto expr = parseExpression();
    if (expr) {
        need(node, expr);
    } else {
        SyntaxError err("Diharapkan ekspresi kondisi setelah 'while'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Expression>", true));
        if (peek().type != TokenType::DOSY) {
        synchronize();
        }
    }

    auto dosy = expect(TokenType::DOSY, NodeType::DoSy, "'do'");
    need(node, dosy);
    if (dosy->isError) synchronize();

    auto stmt = parseStatement();
    if (stmt) {
        need(node, stmt);
    } else {
        SyntaxError err("Diharapkan statement setelah 'do'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Statement>", true));
        synchronize();
    }

    return node;
}

// 30 REPEAT-STATEMENT-> repeatsy + STATEMENT-LIST + untilsy + EXPRESSION
std::shared_ptr<TreeNode> Parser::parseRepeatStatement(){
    if (peek().type != TokenType::REPEATSY) return nullptr;

    auto node = std::make_shared<TreeNode>(NodeType::RepeatStatement);
    need(node, terminal(TokenType::REPEATSY, NodeType::RepeatSy));

    auto stmtList = parseStatementList();
    if (stmtList) {
        need(node, stmtList);
    } else {
        SyntaxError err("Diharapkan statement di dalam blok 'repeat'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Statement-List>", true));
        synchronize();
    }

    auto untilsy = expect(TokenType::UNTILSY, NodeType::UntilSy, "'until'");
    need(node, untilsy);
    if (untilsy->isError) synchronize();

    auto expr = parseExpression();
    if (expr) {
        need(node, expr);
    } else {
        SyntaxError err("Diharapkan ekspresi kondisi setelah 'until'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Expression>", true));
        synchronize();
    }

    return node;
}

// 31 FOR-STATEMENT -> forsy + ident + becomes + EXPRESSION + ( tosy | downtosy) + EXPRESSION + dosy + STATEMENT
std::shared_ptr<TreeNode> Parser::parseForStatement()
{
    if (peek().type != TokenType::FORSY) {
        return nullptr;
    }
    
    auto node = std::make_shared<TreeNode>(NodeType::ForStatement);
    need(node, terminal(TokenType::FORSY, NodeType::ForSy));

    auto iden = expect(TokenType::IDENT, NodeType::Ident, "variabel iterator");
    need(node, iden);
    if (iden->isError) {
        synchronize();
    }
    
    auto become = expect(TokenType::BECOMES, NodeType::Becomes, "':='");
    need(node, become);
    if (become->isError) {
        synchronize();
    }
    
    auto startExpression = parseExpression();
    if (startExpression) {
        need(node, startExpression);
    } else {
        SyntaxError err("Diharapkan ekspresi nilai awal setelah ':='", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Start-Expression>", true));
        synchronize();
    }
    
    if (peek().type == TokenType::TOSY) {
        need(node, terminal(TokenType::TOSY, NodeType::ToSy));
    } else if (peek().type == TokenType::DOWNTOSY) {
        need(node, terminal(TokenType::DOWNTOSY, NodeType::DownToSy));
    } else {
        SyntaxError err("Diharapkan 'to' atau 'downto'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing To/Downto>", true));

    }

    auto endExpression = parseExpression();
    if (endExpression) {
        need(node, endExpression);
    } else {
        SyntaxError err("Diharapkan ekspresi batas akhir", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing End-Expression>", true));
        synchronize();
    }

    auto dosy = expect(TokenType::DOSY, NodeType::DoSy, "'do'");
    need(node, dosy);
    if (dosy->isError) {
        synchronize();
    }

    auto statement = parseStatement();
    if (statement) {
        need(node, statement);
    } else {
        SyntaxError err("Diharapkan statement setelah 'do'", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Statement>", true));
        synchronize();
    }

    return node;
}

// 32 PROCEDURE/FUNCTION-CALL ->ident + (lparent + PARAMETER-LIST + rparent)?
std::shared_ptr<TreeNode> Parser::parseProcedureCall()
{
    if (peek().type != TokenType::IDENT || peek(1).type != TokenType::LPARENT){
        return nullptr;
    }
    
    auto node = std::make_shared<TreeNode>(NodeType::ProcedureCall);
    auto iden = terminal(TokenType::IDENT, NodeType::Ident);
    need(node, iden);
    if (peek().type == TokenType::LPARENT)
    {
        auto lpar = terminal(TokenType::LPARENT, NodeType::LParent);
        need(node, lpar);
        
        auto parameterList = parseParameterList();
        if (parameterList)
        {
            need(node, parameterList);
        }else{
            SyntaxError err("Diharapkan parameter list", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Parameter List>", true));
            synchronize();
        }
        
        auto rpar = expect(TokenType::RPARENT, NodeType::RParent, ")");
        need(node, rpar);
        if (rpar->isError){
            synchronize();
        }
    }


    return node;
}

// 33 PARAMETER-LIST -> EXPRESSION (comma + EXPRESSION)*
std::shared_ptr<TreeNode> Parser::parseParameterList()
{
    auto node = std::make_shared<TreeNode>(NodeType::ParameterList);
    auto firstExpression = parseExpression();
    if (firstExpression)
    {
        need(node, firstExpression);
    }else{
        SyntaxError err("Diharapkan sebuah Expression", peek().line, peek().column);
        syntaxErrors.push_back(err.what());
        node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Expression>", true));
        synchronize();
    }

    while (match(TokenType::COMMA)) {
        auto coma = terminal(TokenType::COMMA, NodeType::Comma);
        need(node, coma);
        if (coma->isError){
            synchronize();
        }
        auto nextExpression = parseExpression();
        if (nextExpression){
            need(node, nextExpression);
        }else{
            SyntaxError err("Diharapkan sebuah Expression seteleh ','", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Expression>", true));
            synchronize();
        }     
    }

    return node;
}

// 34 EXPRESSION -> SIMPLE-EXPRESSION (RELATIONAL-OPERATOR + SIMPLE-EXPRESSION)?
std::shared_ptr<TreeNode> Parser::parseExpression()
{
    auto simpleExpr = parseSimpleExpression();
    if (!simpleExpr) return nullptr;

    auto node = std::make_shared<TreeNode>(NodeType::Expression);
    need(node, simpleExpr);

    // Operator Relasional Opsional (Biasanya hanya 1 kali, tidak diulang)
    auto relOp = parseRelationalOperator();
    if (relOp) {
        need(node, relOp);
        
        auto rightExpr = parseSimpleExpression();
        if (rightExpr) {
            need(node, rightExpr);
        } else {
            SyntaxError err("Diharapkan ekspresi pembanding setelah operator relasional", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing SimpleExpression>", true));
            synchronize();
        }
    }

    return node;
}

// 35 SIMPLE-EXPRESSION -> (plus | minus)? TERM (ADDITIVE-OPERATOR + TERM)*
std::shared_ptr<TreeNode> Parser::parseSimpleExpression()
{
    auto node = std::make_shared<TreeNode>(NodeType::SimpleExpression);

    // Opsional + atau - di awal (Unary)
    if (peek().type == TokenType::PLUS)       need(node, terminal(TokenType::PLUS, NodeType::Plus));
    else if (peek().type == TokenType::MINUS) need(node, terminal(TokenType::MINUS, NodeType::Minus));

    auto term = parseTerm();
    if (!term) return nullptr; // Jika tidak ada awalan, batalkan
    need(node, term);

    // Selama token berikutnya adalah Additive Operator (+, -, or)
    auto addOp = parseAdditiveOperator();
    while (addOp) {
        need(node, addOp);
        
        auto nextTerm = parseTerm();
        if (nextTerm) {
            need(node, nextTerm);
        } else {
            SyntaxError err("Diharapkan operand setelah operator penambahan", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Term>", true));
            synchronize();
        }
        addOp = parseAdditiveOperator();
    }

    return node;
}

// 36 TERM -> FACTOR (MULTIPLICATIVE-OPERATOR + FACTOR)*
std::shared_ptr<TreeNode> Parser::parseTerm()
{
    auto factor = parseFactor();
    if (!factor) return nullptr; // Jika Factor gagal, Term juga gagal (FIRST Set)

    auto node = std::make_shared<TreeNode>(NodeType::Term);
    need(node, factor);

    // Selama token berikutnya adalah Multiplicative Operator (*, /, mod, dll)
    auto multOp = parseMultiplicativeOperator();
    while (multOp) {
        need(node, multOp);
        
        auto nextFactor = parseFactor();
        if (nextFactor) {
            need(node, nextFactor);
        } else {
            SyntaxError err("Diharapkan operand setelah operator perkalian", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Factor>", true));
            synchronize();
        }
        multOp = parseMultiplicativeOperator(); 
    }

    return node;
}

// 37 FACTOR -> (minus)? FACTOR | intcon | charcon | string | (lparent + EXPRESSION + rparent) | notsy + FACTOR | PROCEDURE/FUNCTION-CALL | VARIABLE
std::shared_ptr<TreeNode> Parser::parseFactor()
{
    TokenType t = peek().type;
    auto node = std::make_shared<TreeNode>(NodeType::Factor);

    // Cek ada minus di depan (Unary Minus)
    if (t == TokenType::MINUS) {
        need(node, terminal(TokenType::MINUS, NodeType::Minus)); 
        
        auto innerFactor = parseFactor(); 
        if (innerFactor) {
            need(node, innerFactor);
        } else {
            SyntaxError err("Diharapkan faktor setelah '-'", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Factor>", true));
            synchronize();
        }
        return node;
    }

    // Cek Const
    if (t == TokenType::INTCON)   { need(node, terminal(t, NodeType::IntCon)); return node; }
    if (t == TokenType::REALCON)  { need(node, terminal(t, NodeType::RealCon)); return node; }
    if (t == TokenType::CHARCON)  { need(node, terminal(t, NodeType::CharCon)); return node; }
    if (t == TokenType::STRINGCON){ need(node, terminal(t, NodeType::String)); return node; }

    // Cek (lparent + expression + rparent)
    if (t == TokenType::LPARENT) {
        need(node, terminal(TokenType::LPARENT, NodeType::LParent));
        
        auto expr = parseExpression();
        if (expr) need(node, expr);
        else {
            SyntaxError err("Diharapkan ekspresi di dalam kurung", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Expression>", true));
            synchronize();
        }
        
        auto rparent = expect(TokenType::RPARENT, NodeType::RParent, "')'");
        need(node, rparent);
        if (rparent->isError) synchronize();
        return node;
    }

    // Cek (notsy + factor)
    if (t == TokenType::NOTSY) {
        need(node, terminal(TokenType::NOTSY, NodeType::NotSy));
        
        auto innerFactor = parseFactor();
        if (innerFactor) need(node, innerFactor);
        else {
            SyntaxError err("Diharapkan faktor setelah 'not'", peek().line, peek().column);
            syntaxErrors.push_back(err.what());
            node->addChild(std::make_shared<TreeNode>(NodeType::Error, "<Missing Factor>", true));
            synchronize();
        }
        return node;
    }

    // Cek Identifier: Procedure/Function Call ATAU Variable
    // Karena keduanya diawali IDENT, ini SATU-SATUNYA tempat kita butuh save() & restore()
    if (t == TokenType::IDENT) {
        size_t saved = save(); 
        
        // Cek Procedure Call dulu
        auto callNode = parseProcedureCall();
        if (callNode) {
            need(node, callNode);
            return node;
        }
        
        restore(saved); // Kalau gagal (bukan procedure call), kembalikan token
        
        // Cek Variabel
        auto varNode = parseVariable();
        if (varNode) {
            need(node, varNode);
            return node;
        }
        
        restore(saved);
    }

    return nullptr; 
}

// 38 RELATIONAL-OPERATOR -> eql | neq | gtr | geq | lss | leq
std::shared_ptr<TreeNode> Parser::parseRelationalOperator()
{
    TokenType t = peek().type;
    auto node = std::make_shared<TreeNode>(NodeType::RelationalOperator);

    if      (t == TokenType::EQL) need(node, terminal(t, NodeType::Eql));
    else if (t == TokenType::NEQ) need(node, terminal(t, NodeType::Neq));
    else if (t == TokenType::GTR) need(node, terminal(t, NodeType::Gtr));
    else if (t == TokenType::GEQ) need(node, terminal(t, NodeType::Geq));
    else if (t == TokenType::LSS) need(node, terminal(t, NodeType::Lss));
    else if (t == TokenType::LEQ) need(node, terminal(t, NodeType::Leq));
    else return nullptr;

    return node;
}

// 39 ADDITIVE-OPERATOR -> plus | minus | orsy
std::shared_ptr<TreeNode> Parser::parseAdditiveOperator()
{
    TokenType t = peek().type;
    auto node = std::make_shared<TreeNode>(NodeType::AdditiveOperator);

    if      (t == TokenType::PLUS)  need(node, terminal(t, NodeType::Plus));
    else if (t == TokenType::MINUS) need(node, terminal(t, NodeType::Minus));
    else if (t == TokenType::ORSY)  need(node, terminal(t, NodeType::OrSy));
    else return nullptr;

    return node;
}

// 40 MULTIPLICATIVE-OPERATOR -> times | rdiv | idiv | imod | andsy
std::shared_ptr<TreeNode> Parser::parseMultiplicativeOperator()
{
    TokenType t = peek().type;
    auto node = std::make_shared<TreeNode>(NodeType::MultiplicativeOperator);

    if      (t == TokenType::TIMES) need(node, terminal(t, NodeType::Times));
    else if (t == TokenType::RDIV)  need(node, terminal(t, NodeType::RDiv));
    else if (t == TokenType::IDIV)  need(node, terminal(t, NodeType::IDiv));
    else if (t == TokenType::IMOD)  need(node, terminal(t, NodeType::IMod));
    else if (t == TokenType::ANDSY) need(node, terminal(t, NodeType::AndSy));
    else return nullptr;

    return node;
}

// Revisi
std::shared_ptr<TreeNode> Parser::parseVariable()
{
    size_t saved = save();
    auto node = std::make_shared<TreeNode>(NodeType::Variable);
    if (!need(node, terminal(TokenType::IDENT, NodeType::Ident))) return fail(saved);
    while (need(node, parseComponentVariable())) {
    }
    return node;
}

std::shared_ptr<TreeNode> Parser::parseIndexList()
{
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

std::shared_ptr<TreeNode> Parser::parseComponentVariable()
{
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

const Token &Parser::peek(int offset) const{
    if (pos + offset >= tokens.size()) {
        if (!tokens.empty()) {
            return tokens.back();
        }
        static Token safeToken{TokenType::PERIOD, "EOF", 0, 0};
        return safeToken;
    }
    return tokens[pos + offset];
}

const Token &Parser::advance(){
    return tokens[pos++];
}

std::string Parser::printNode(const std::shared_ptr<TreeNode> &node){
    if (node == nullptr) {
        return "";
    }

    std::string label = node->typeString();
    if (!node->value.empty()) {
        label += "(" + node->value + ")";
    }
    return label;
}

void Parser::printParseTree(const std::shared_ptr<TreeNode> &node, const std::string &prefix = "", bool last = true, bool root = true)
{
    if (node == nullptr) {
        return;
    }

    if (root) {
        std::cout << printNode(node) << '\n';
    }
    else {
        std::cout << prefix << (last ? "└── " : "├── ") << printNode(node) << '\n';
    }

    std::string childPrefix = prefix;
    if (!root) {
        childPrefix += last ? "    " : "│   ";
    }

    for (size_t i = 0; i < node->children.size(); ++i) {
        printParseTree(node->children[i], childPrefix, i + 1 == node->children.size(), false);
    }
}