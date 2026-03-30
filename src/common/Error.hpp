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


// Continue for Class SyntaxError and SemanticError here inherit from CompilerError