#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "LexerState.hpp"
#include "../common/Token.hpp"

class LexicalAnalyzer
{
private:
    LexerState state;

    // Loopup table untuk memetekan string "var", "begin" ke Tokentype
    std::unordered_map<std::string, TokenType> keywordsMap;

    // Menyimpan semua error
    std::vector<Token> errorTokens;

    // Fungsi internal untuk mendaftarkan semua keyword ke dalam map saat inisialisasi
    void initKeywordsMap();

    // Fungsi utilitas untuk melewati spasi, tab, dan newline (Whitespace)
    void skipWhitespace();

public:
    LexicalAnalyzer(const std::string& sourceCode);

    // CORE METHOD: Global DFA Dispatcher
    // Fungsi ini akan dipanggil berulang kali untuk mendapatkan token satu per satu
    Token getNextToken();


    // Wrapper helper: Langsung mengubah seluruh source code menjadi daftar token
    // Berguna untuk meng-output hasil ke file output.txt
    std::vector<Token> tokenizeAll();

    // buat post eof print errors
    const std::vector<Token>& getErrorTokens() const;
    
    ~LexicalAnalyzer();
};
