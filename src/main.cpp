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
    std::string printNode(const std::shared_ptr<TreeNode> &node)
    {
        if (node == nullptr)
        {
            return "";
        }

        std::string label = node->typeString();
        if (!node->value.empty())
        {
            label += "(" + node->value + ")";
        }

        return label;
    }

    void printParseTree(const std::shared_ptr<TreeNode> &node, const std::string &prefix = "", bool last = true, bool root = true)
    {
        if (node == nullptr)
        {
            return;
        }

        if (root)
        {
            std::cout << printNode(node) << '\n';
        }
        else
        {
            std::cout << prefix << (last ? "└── " : "├── ") << printNode(node) << '\n';
        }

        std::string childPrefix = prefix;
        if (!root)
        {
            childPrefix += last ? "    " : "│   ";
        }

        for (size_t i = 0; i < node->children.size(); ++i)
        {
            printParseTree(node->children[i], childPrefix, i + 1 == node->children.size(), false);
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

        std::string inputPat;
        outputPath = "output/output.txt";
        
        int filenum;
        int typeChoice;

        std::cout << "1. Lexer" << std::endl;
        std::cout << "2. Parser" << std::endl;

        std::cout << "Choose Type: ";
        std::cin >> typeChoice;

        std::cout << "Choose Testcase: ";
        std::cin >> filenum;

        switch (typeChoice)
        {
        case 1:
            inputPath = "test/lexer/" + std::to_string(filenum) + ".arion";
            break;
        case 2:
            inputPath = "test/parser/" + std::to_string(filenum) + ".arion";
            break;
        default:
            break;
        }

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
