#include "Error.hpp"

CompilerError::CompilerError(const std::string &msg, int l, int c, const std::string phase) :
        message(msg), line(l), column(c) {
            // Format pesan: [Lexical Error] at Line 10, Col 5: Invalid character '@'
        fullMessage = "[" + phase + "] at Line " + std::to_string(line) + 
                      ", Col " + std::to_string(column) + ": " + message;
    }


// TODO : Implement Destructor
CompilerError::~CompilerError() = default;

LexicalError::~LexicalError() = default;