#include "Writer.hpp"

#include <fstream>
#include <stdexcept>

bool printWithValue(TokenType type)
{
    return type == TokenType::IDENT ||
           type == TokenType::INTCON ||
           type == TokenType::REALCON ||
           type == TokenType::CHARCON ||
           type == TokenType::STRINGCON ||
           type == TokenType::COMMENT;
}

std::string getTokenName(TokenType type)
{

    // idk if charcon supposed to be in '' too or not
    // if (type == TokenType::CHARCON)
    // {
    //     return "char";
    // }
    if (type == TokenType::STRINGCON)
    {
        return "string";
    }

    return tokenTypeToString(type);
}

void writeOutput(const std::vector<Token> &tokens, const std::string &filePath)
{
    std::ofstream outputFile(filePath);
    if (!outputFile.is_open())
    {
        throw std::runtime_error("Failed to write " + filePath);
    }

    for (const auto &token : tokens)
    {
        std::string tokenName = getTokenName(token.type);

        if (token.type == TokenType::STRINGCON)
        {
            outputFile << tokenName << " (\'" << token.value << "\')\n";
        }
        else if (token.type == TokenType::COMMENT)
        {
            outputFile << tokenName << " (\'" << token.value << "\')\n";
        }
        else if (printWithValue(token.type))
        {
            outputFile << tokenName << " (" << token.value << ")\n";
        }
        else
        {
            outputFile << tokenName << "\n";
        }
    }
}