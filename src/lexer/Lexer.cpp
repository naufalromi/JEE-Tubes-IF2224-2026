#include "Lexer.hpp"
#include "../common/Error.hpp"
#include "SpecificScanners.hpp"
#include "iostream"
void LexicalAnalyzer::initKeywordsMap()
{
    // Keywords bawaan
    keywordsMap["const"] = TokenType::CONSTSY;
    keywordsMap["type"] = TokenType::TYPESY;
    keywordsMap["var"] = TokenType::VARSY;
    keywordsMap["function"] = TokenType::FUNCTIONSY;
    keywordsMap["procedure"] = TokenType::PROCEDURESY;
    keywordsMap["array"] = TokenType::ARRAYSY;
    keywordsMap["record"] = TokenType::RECORDSY;
    keywordsMap["program"] = TokenType::PROGRAMSY;
    keywordsMap["begin"] = TokenType::BEGINSY;
    keywordsMap["if"] = TokenType::IFSY;
    keywordsMap["case"] = TokenType::CASESY;
    keywordsMap["repeat"] = TokenType::REPEATSY;
    keywordsMap["while"] = TokenType::WHILESY;
    keywordsMap["for"] = TokenType::FORSY;
    keywordsMap["end"] = TokenType::ENDSY;
    keywordsMap["else"] = TokenType::ELSESY;
    keywordsMap["until"] = TokenType::UNTILSY;
    keywordsMap["of"] = TokenType::OFSY;
    keywordsMap["do"] = TokenType::DOSY;
    keywordsMap["to"] = TokenType::TOSY;
    keywordsMap["downto"] = TokenType::DOWNTOSY;
    keywordsMap["then"] = TokenType::THENSY;

    //  5 Word-Operators (Operator berbentuk kata)
    keywordsMap["not"] = TokenType::NOTSY;
    keywordsMap["div"] = TokenType::IDIV;
    keywordsMap["mod"] = TokenType::IMOD;
    keywordsMap["and"] = TokenType::ANDSY;
    keywordsMap["or"] = TokenType::ORSY;
}

void LexicalAnalyzer::skipWhitespace()
{
    while (!state.isAtEnd() && std::isspace(state.peek())) {
        state.advance();
    }
}

LexicalAnalyzer::LexicalAnalyzer(const std::string &sourceCode) : state(sourceCode)
{
    initKeywordsMap();
};

// TODO : Implementasi Destructor
LexicalAnalyzer::~LexicalAnalyzer() {

};

Token LexicalAnalyzer::getNextToken()
{
    skipWhitespace();
    if (state.isAtEnd()) {
        return Token(TokenType::END_OF_FILE, "EOF", state.currentLine, state.currentColumn);
    }

    char currentChar = state.peek();
    Token token(TokenType::ERROR_TOKEN, "", 0, 0);

    // Berada di state 0
    std::cout << "State 0 => ";
    if (std::isalpha(currentChar) || currentChar == '_') {
        token = SpecificScanners::scanAlpha(state, keywordsMap);
    }
    else if (std::isdigit(currentChar)) {
        // token = SpecificScanners::scanNumeric(state);
    }
    else if (currentChar == '\'') {
        // token = SpecificScanners::scanText(state);
    }
    else {
        token = SpecificScanners::scanSymbol(state);
        if (token.value.length() == 1) {
             std::cout << token.value << " => char => State 0 => Gotten: " 
                       << tokenTypeToString(token.type) << "(" << token.value << ")\n";
        }
    }

    if (token.type == TokenType::ERROR_TOKEN) {
        throw LexicalError("Karakter illegal atau token tidak valid: '" + token.value + "'", token.line, token.column);
    }

    return token;
}

// TODO : Implementasi Langsung mengubah seluruh source code menjadi daftar token
std::vector<Token> LexicalAnalyzer::tokenizeAll()
{
    std::vector<Token> tokens;
    Token currentToken = getNextToken();

    while (currentToken.type != TokenType::END_OF_FILE)
    {
        tokens.push_back(currentToken);
        currentToken = getNextToken();
    }

    // Memasukan EOF
    tokens.push_back(currentToken);
    
    return tokens;
}