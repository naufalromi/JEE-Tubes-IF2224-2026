#include "Error.hpp"

// Error lookup table so its universal
std::string errorTypeToMessage(ErrorType type, const std::string &detail)
{
    switch (type)
    {
        case ErrorType::IllegalChar:
            return "illegal character" + (detail.empty() ? "" : ": " + detail);
        case ErrorType::IncompleteReal:
            return "incomplete real" + (detail.empty() ? "" : ": " + detail);
        case ErrorType::UnterminatedString:
            return "unterminated string" + (detail.empty() ? "" : ": " + detail);
        case ErrorType::UnterminatedComment:
            return "unterminated comment" + (detail.empty() ? "" : ": " + detail);
        case ErrorType::None:
        default:
            return detail.empty() ? "unknown lexical error" : detail;
    }
}

CompilerError::CompilerError(const std::string &msg, int l, int c, const std::string phase) :
        message(msg), line(l), column(c) {
            // Format pesan: [Lexical Error] at Line 10, Col 5: Invalid character '@'
        fullMessage = "[" + phase + "] at Line " + std::to_string(line) + 
                      ", Col " + std::to_string(column) + ": " + message;
    }


// TODO : Implement Destructor
CompilerError::~CompilerError() = default;

LexicalError::~LexicalError() = default;