#pragma once

#include <string>

class CompilerError : public std::exception {
protected:
    std::string message;
    int line;
    int column;
    std::string fullMessage;

public:
    // Format pesan: [Lexical Error] at Line 10, Col 5: Invalid character '@'
    CompilerError(const std::string &msg, int l, int c, const std::string phase);

    // Override fungsi what() dari std::exception
    virtual const char *what() const noexcept override
    {
        return fullMessage.c_str();
    }

    int getLine() const { return line; }
    int getColumn() const { return column; }

    ~CompilerError();
};

class LexicalError : public CompilerError {
public:
    LexicalError(const std::string &msg, int l, int c)
        : CompilerError(msg, l, c, "Lexical Error") {};
    ~LexicalError();
};

// for easier error documentation
enum class ErrorType {
    None,
    IllegalChar,
    IncompleteReal,
    UnterminatedString,
    UnterminatedComment
};

std::string errorTypeToMessage(ErrorType type, const std::string &detail = "");

// Continue for Class SyntaxError and SemanticError here inherit from CompilerError

class SyntaxError : public CompilerError{
    public:
    SyntaxError(const std::string &msg, int l, int c)
        : CompilerError(msg, l, c, "Syntax Error") {};
    ~SyntaxError();
};

class SemanticError : public CompilerError{
    SemanticError(const std::string &msg, int l, int c)
        : CompilerError(msg, l, c, "Semantic Error") {};
    ~SemanticError();
};
