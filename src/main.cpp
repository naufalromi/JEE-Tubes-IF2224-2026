#include "common/Error.hpp"
#include "common/Reader.hpp"
#include "common/Writer.hpp"
#include "lexer/Lexer.hpp"
#include <fstream>
#include <iostream>

int main()
{
    while (true) {
        /* code */

        std::string inputPath, outputPath;

        std::cout << "Source code file path: ";
        std::cin >> inputPath;

        std::cout << "Output file path: ";
        std::cin >> outputPath;

        // std::string sourceCode = "program Hallo\n";
        try {
            std::string sourceCode = readFile(inputPath);
            LexicalAnalyzer lexer(sourceCode);
            std::vector<Token> tokens = lexer.tokenizeAll();
            const std::vector<Token> &errorTokens = lexer.getErrorTokens();
            // for (auto &&token : tokens)
            // {
            //     std::cout << token.value << ": "<< tokenTypeToString(token.type) << "\n";
            // }

            writeOutput(tokens, outputPath);

            if (!errorTokens.empty()) {
                std::cout << "\nLexical errors found:\n";
                for (const auto &errorToken : errorTokens) {
                    std::cout << "- Line " << errorToken.line << ", Col " << errorToken.column << ": " << errorTypeToMessage(errorToken.errorType, errorToken.value) << "\n";
                }
                return 0;
            }

            std::cout << "berhasil kah? \n";
            return 0;
        }
        catch (const std::exception &e) {
            std::cerr << "Terjadi Kesalahan Sistem: " << e.what() << "\n";
            return 1;
        }
    }
}