#include "common/Error.hpp"
#include "common/Reader.hpp"
#include "common/Writer.hpp"
#include "lexer/Lexer.hpp"
#include "syntax/Parser.hpp"
#include "semantic/ASTBuilder.hpp"
#include "semantic/ASTPrinter.hpp"
#include <fstream>
#include <iostream>
#include <memory>


int main()
{
    while (true)
    {
        std::string inputPath, outputPath;
        outputPath = "output/output.txt";
        
        int filenum;
        int typeChoice;

        std::cout << "1. Lexical Analyzer" << std::endl;
        std::cout << "2. Syntax Analyzer" << std::endl;
        std::cout << "3. Semantic Analyzer" << std::endl;
        std::cout << "Choose Type: ";
        std::cin >> typeChoice;
        std::cout << "Choose Testcase: ";
        std::cin >> filenum;

        if (typeChoice == 1) {
            inputPath = "test/lexer/" + std::to_string(filenum) + ".arion";
        } 
        else if (typeChoice == 2) {
            inputPath = "test/semantic/" + std::to_string(filenum) + ".arion";
        } 
        else if (typeChoice == 3) {
            inputPath = "test/parser/" + std::to_string(filenum) + ".arion";
        }

        try
        {
            std::string sourceCode = readFile(inputPath);
            LexicalAnalyzer lexer(sourceCode);
            std::vector<Token> tokens = lexer.tokenizeAll();
            const std::vector<Token> &errorTokens = lexer.getErrorTokens();

            if (typeChoice == 1) {
                writeOutput(tokens, outputPath);
            }

            if (!errorTokens.empty())
            {
                std::cout << "\n[!] Lexical errors found:\n";
                for (const auto &errorToken : errorTokens)
                {
                    std::cout << "- Line " << errorToken.line << ", Col " << errorToken.column << ": "
                              << errorTypeToMessage(errorToken.errorType, errorToken.value) << "\n";
                }
                return 1;
            }

            if (typeChoice == 1) {
                std::cout << "Lexer berhasil!\n";
                return 0;
            }

            Parser parser(tokens);
            std::shared_ptr<TreeNode> parseTree = parser.parseProgram();

            if (!parser.syntaxErrors.empty()) {
                std::cout << "\n[!] Syntax errors found:\n";
                for (const auto& err : parser.syntaxErrors) {
                    std::cerr << "- " << err << "\n";
                }
                return 1;
            }

            if (parseTree == nullptr) {
                std::cerr << "Parse failed (Tree is null).\n";
                return 1;
            }

            if (typeChoice == 2) {
                std::cout << "Parse tree berhasil dibuat:\n";
                Parser::printParseTree(parseTree, "", true, true);
                return 0;
            }

            if (typeChoice == 3) {
                ASTBuilder builder(parseTree);
                
                std::shared_ptr<ProgramNode> ast = builder.build();

                if (ast) {
                    std::cout << "AST berhasil dibuat! Berikut strukturnya:\n\n";
                    ASTPrinter printer;
                    printer.print(ast);
                } else {
                    std::cout << "Gagal membangun AST!\n";
                }
                return 0;
            }

            std::cout << "Invalid choice\n";
            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Terjadi Kesalahan Sistem: " << e.what() << "\n";
            return 1;
        }
    }
}
