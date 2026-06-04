#include "common/Error.hpp"
#include "common/Reader.hpp"
#include "common/Writer.hpp"
#include "lexer/Lexer.hpp"
#include "syntax/Parser.hpp"
#include "semantic/ASTBuilder.hpp"
#include "semantic/ASTPrinter.hpp"
#include "semantic/SemanticAnalyzer.hpp"
#include "codegen/CodeGenerator.hpp"
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
        std::cout << "4. Intermediate Code Generator" << std::endl;
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
            inputPath = "test/semantic/" + std::to_string(filenum) + ".arion";
        } 
        else if (typeChoice == 4) {
            inputPath = "test/semantic/" + std::to_string(filenum) + ".arion";
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

            if (typeChoice >= 3) {
                SemanticAnalyzer analyzer(parseTree);
                std::shared_ptr<ProgramNode> decoratedAST = analyzer.analyse();

                if (!decoratedAST) {
                    std::cout << "[!] Semantic analysis failed!\n";
                    return 1;
                }

                if (analyzer.hasErrors()) {
                    std::cout << "[!] Semantic analysis completed with errors.\n";
                    if (typeChoice == 4) {
                        std::cout << "[!] Cannot generate code with semantic errors.\n";
                        return 1;
                    }
                }

                if (typeChoice == 3) {
                    int displayChoice;
                    std::cout << "\n========== DISPLAY OPTIONS ==========\n";
                    std::cout << "1. Decorated AST only\n";
                    std::cout << "2. Symbol Table (tab) only\n";
                    std::cout << "3. Array Table (atab) only\n";
                    std::cout << "4. Block Table (btab) only\n";
                    std::cout << "5. All semantic information\n";
                    std::cout << "6. Exit\n";
                    std::cout << "Choose: ";
                    std::cin >> displayChoice;

                    switch (displayChoice) {
                        case 1: analyzer.printDecoratedAST(); break;
                        case 2: analyzer.printSymbolTable(); break;
                        case 3: analyzer.printArrayTable(); break;
                        case 4: analyzer.printBlockTable(); break;
                        case 5: analyzer.printAllSemanticInfo(); break;
                        case 6: break;
                        default: std::cout << "Invalid choice\n";
                    }
                    return 0;
                }

                if (typeChoice == 4) {
                    const SymbolTable* symTab = &(analyzer.getSymbolTable());
                    CodeGenerator codegen(symTab);
                    codegen.generate(decoratedAST.get());
                    std::string bytecodePath = inputPath.substr(0, inputPath.find_last_of('.')) + ".aobj";
                    if (codegen.exportToFile(bytecodePath)) {
                        std::cout << "\n[+] Success generated object file to: " << bytecodePath << "\n";
                    } else {
                        std::cout << "\n[-] Failed to save object file.\n";
                        return 1;
                    }
                    
                    return 0;
                }
            }

            std::cout << "Invalid choice\n";
            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "System error: " << e.what() << "\n";
            return 1;
        }
    }
}
