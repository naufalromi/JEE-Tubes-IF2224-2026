#pragma once
#include <string>
#include "Error.hpp"

enum class TokenType {
    // ---------------------------------------------------------
    // 1. Keywords / Reserved Words
    // ---------------------------------------------------------
    PROGRAMSY, CONSTSY, TYPESY, VARSY, FUNCTIONSY, PROCEDURESY,
    ARRAYSY, RECORDSY, BEGINSY, IFSY, CASESY, REPEATSY, WHILESY,
    FORSY, ENDSY, ELSESY, UNTILSY, OFSY, DOSY, TOSY, DOWNTOSY, THENSY,

    // ---------------------------------------------------------
    // 2. Word Operators (Operator berbasis kata)
    // ---------------------------------------------------------
    NOTSY, IDIV, IMOD, ANDSY, ORSY,

    // ---------------------------------------------------------
    // 3. Identifiers & Constants
    // ---------------------------------------------------------
    IDENT,          // Variabel/Nama fungsi (misal: x, PI, MyInt)
    INTCON,         // Konstanta integer (misal: 1, 48)
    REALCON,        // Konstanta riil (misal: 3.14)
    CHARCON,        // Konstanta karakter (misal: 'j')
    STRINGCON,      // Konstanta string (misal: 'Hello'). 

    // ---------------------------------------------------------
    // 4. Relational & Assignment Operators (Simbol Multi-Karakter)
    // ---------------------------------------------------------
    EQL,            // ==
    NEQ,            // <>
    GTR,            // >
    GEQ,            // >=
    LSS,            // <
    LEQ,            // <=
    BECOMES,        // :=

    // ---------------------------------------------------------
    // 5. Arithmetic Operators & Punctuation (Simbol 1 Karakter)
    // ---------------------------------------------------------
    PLUS,           // +
    MINUS,          // -
    TIMES,          // *
    RDIV,           // /
    LPARENT,        // (
    RPARENT,        // )
    LBRACK,         // [
    RBRACK,         // ]
    COMMA,          // ,
    SEMICOLON,      // ;
    PERIOD,         // .
    COLON,          // :

    // ---------------------------------------------------------
    // 6. Comments 
    // ---------------------------------------------------------
    COMMENT,        // { ... } atau (* ... *)

    // ---------------------------------------------------------
    // 7. Special Control Tokens
    // ---------------------------------------------------------
    END_OF_FILE,    // Penanda akhir file
    ERROR_TOKEN     // Penanda jika terjadi lexical error (misal ada karakter '@')
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    ErrorType errorType;

    Token(TokenType t, std::string v, int l, int c)
        : type(t), value(v), line(l), column(c), errorType(ErrorType::None) {}

    // Specific token constructor for errors
    Token(TokenType t, ErrorType e, int l, int c, std::string detail = "")
        : type(t), value(detail), line(l), column(c), errorType(e) {}

    
    
};
std::string tokenTypeToString(TokenType type);

