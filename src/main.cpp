#include <iostream>
#include <fstream>
#include "lexer/Lexer.hpp"
#include "common/Error.hpp"

int main(){
    std::string sourceCode = "program Hallo\n";
    try
    {
        LexicalAnalyzer lexer(sourceCode);
        std::vector<Token> tokens = lexer.tokenizeAll();
        for (auto &&token : tokens)
        {
            std::cout << token.value << ": "<< tokenTypeToString(token.type) << "\n";
        }
        
        std::cout << "berhasil kah? \n";
    }
    catch(const LexicalError& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    catch(const std::exception& e){
        std::cerr << "Terjadi Kesalahan Sistem: " << e.what() << "\n";
        return 1;
    }
    
}