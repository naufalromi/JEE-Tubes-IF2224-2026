#include "common/Error.hpp"
#include "common/Reader.hpp"
#include "common/Writer.hpp"
#include "lexer/Lexer.hpp"
#include "syntax/Parser.hpp"
#include <fstream>
#include <iostream>
#include <memory>

namespace
{
    void printParseTree(const std::shared_ptr<TreeNode> &node, int depth = 0)
    {
        if (node == nullptr)
        {
            return;
        }

        std::cout << std::string(depth * 2, ' ') << node->typeString();
        if (!node->value.empty())
        {
            std::cout << " (" << node->value << ")";
        }
        std::cout << '\n';

        for (const auto &child : node->children)
        {
            printParseTree(child, depth + 1);
        }
    }
}

int main()
{
    while (true)
    {
        /* code */

        std::string inputPath, outputPath;

        // std::cout << "Source code file path: ";
        // std::cin >> inputPath;

        // std::cout << "Output file path: ";
        // std::cin >> outputPath;

        inputPath = "test/2TestConst.arion";
        outputPath = "output/output.txt";

        int typeChoice;
        std::cout << "1. Lexer" << std::endl;
        std::cout << "2. Parser" << std::endl;
        std::cout << "Choose Type: ";
        std::cin >> typeChoice;

        // std::string sourceCode = "program Hallo\n";
        try
        {
            if (typeChoice == 2)
            {
                std::string sourceCode = readFile(inputPath);
                LexicalAnalyzer lexer(sourceCode);
                std::vector<Token> tokens = lexer.tokenizeAll();
                const std::vector<Token> &errorTokens = lexer.getErrorTokens();

                if (!errorTokens.empty())
                {
                    std::cout << "\nLexical errors found:\n";
                    for (const auto &errorToken : errorTokens)
                    {
                        std::cout << "- Line " << errorToken.line << ", Col " << errorToken.column << ": "
                                  << errorTypeToMessage(errorToken.errorType, errorToken.value) << "\n";
                    }
                    return 1;
                }

                Parser parser(tokens);
                std::shared_ptr<TreeNode> parseTree = parser.parseProgram();

                // if (parseTree == nullptr || !parser.match(TokenType::END_OF_FILE))
                // {
                //     std::cerr << "Parse failed.\n";
                //     return 1;
                // }

                std::cout << "Parse tree:\n";
                printParseTree(parseTree);

                return 0;
            }
            else if (typeChoice == 1)
            {
                std::string sourceCode = readFile(inputPath);
                LexicalAnalyzer lexer(sourceCode);
                std::vector<Token> tokens = lexer.tokenizeAll();
                const std::vector<Token> &errorTokens = lexer.getErrorTokens();
                // for (auto &&token : tokens)
                // {
                //     std::cout << token.value << ": "<< tokenTypeToString(token.type) << "\n";
                // }

                writeOutput(tokens, outputPath);

                if (!errorTokens.empty())
                {
                    std::cout << "\nLexical errors found:\n";
                    for (const auto &errorToken : errorTokens)
                    {
                        std::cout << "- Line " << errorToken.line << ", Col " << errorToken.column << ": " << errorTypeToMessage(errorToken.errorType, errorToken.value) << "\n";
                    }
                    return 0;
                }

                std::cout << "berhasil kah? \n";
            }
            
            std::cout << "Invalid";
            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Terjadi Kesalahan Sistem: " << e.what() << "\n";
            return 1;
        }
    }
}
