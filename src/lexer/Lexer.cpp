#include "Lexer.hpp"
#include "../common/Error.hpp"
#include "SpecificScanners.hpp"
#include "iostream"


void LexicalAnalyzer::initKeywordsArray(){
    keywordsArray = {
        "and",
        "array",
        "begin",
        "case",
        "const",
        "div",
        "do",
        "downto",
        "else",
        "end",
        "for",
        "function",
        "if",
        "mod",
        "not",
        "of",
        "or",
        "procedure",
        "program",
        "record",
        "repeat",
        "then",
        "to",
        "type",
        "until",
        "var",
        "while"
    };

    tokenTypeArray = {
        TokenType::ANDSY,       // "and"
        TokenType::ARRAYSY,     // "array"
        TokenType::BEGINSY,     // "begin"
        TokenType::CASESY,      // "case"
        TokenType::CONSTSY,     // "const"
        TokenType::IDIV,        // "div"
        TokenType::DOSY,        // "do"
        TokenType::DOWNTOSY,    // "downto"
        TokenType::ELSESY,      // "else"
        TokenType::ENDSY,       // "end"
        TokenType::FORSY,       // "for"
        TokenType::FUNCTIONSY,  // "function"
        TokenType::IFSY,        // "if"
        TokenType::IMOD,        // "mod"
        TokenType::NOTSY,       // "not"
        TokenType::OFSY,        // "of"
        TokenType::ORSY,        // "or"
        TokenType::PROCEDURESY, // "procedure"
        TokenType::PROGRAMSY,   // "program"
        TokenType::RECORDSY,    // "record"
        TokenType::REPEATSY,    // "repeat"
        TokenType::THENSY,      // "then"
        TokenType::TOSY,        // "to"
        TokenType::TYPESY,      // "type"
        TokenType::UNTILSY,     // "until"
        TokenType::VARSY,       // "var"
        TokenType::WHILESY      // "while"
    };
}

void LexicalAnalyzer::skipWhitespace()
{
    while (!state.isAtEnd() && std::isspace(state.peek())) {
        state.advance();
    }
}

LexicalAnalyzer::LexicalAnalyzer(const std::string &sourceCode) : state(sourceCode)
{
    initKeywordsArray();
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

    // why start as error token? well ig yeah
    Token token(TokenType::ERROR_TOKEN, ErrorType::None, 0, 0);

    // Berada di state 0
    std::cout << "State 0 => ";
    if (std::isalpha(static_cast<unsigned char>(currentChar))) {
        token = SpecificScanners::scanAlpha(state, keywordsArray, tokenTypeArray);
    }
    else if (std::isdigit(currentChar)) {
        token = SpecificScanners::scanNumeric(state);
    }
    else if (currentChar == '\'') {
        token = SpecificScanners::scanText(state);
    }
    else {
        token = SpecificScanners::scanSymbol(state);

        if (token.value.length() == 1) {
             std::cout << token.value << " => char => State 0 => Gotten: " 
                       << tokenTypeToString(token.type) << "(" << token.value << ")\n";
        }
    }

    return token;
}

// TODO : Implementasi Langsung mengubah seluruh source code menjadi daftar token
std::vector<Token> LexicalAnalyzer::tokenizeAll()
{
    errorTokens.clear();
    std::vector<Token> tokens;
    Token currentToken = getNextToken();

    while (currentToken.type != TokenType::END_OF_FILE)
    {
        currentToken.type == TokenType::ERROR_TOKEN ? errorTokens.push_back(currentToken) : tokens.push_back(currentToken);
        currentToken = getNextToken();
    }

    // Memasukan EOF
    tokens.push_back(currentToken);
    
    return tokens;
}

const std::vector<Token>& LexicalAnalyzer::getErrorTokens() const
{
    return errorTokens;
}