#include "SpecificScanners.hpp"
#include <cctype> // Untuk std::isalnum, std::isalpha, std::isdigit
#include <iostream>

namespace SpecificScanners {
    // Alphanumeric (Identifier, Keyword)
    Token scanAlpha(LexerState &state, const std::unordered_map<std::string, TokenType> &keywordsMap)
    {
        int startLine = state.currentLine;
        int startColumn = state.currentColumn;
        int stateCount = 1;
        bool first = true;
        std::string lexeme = "";

        // Looping ini adalah State 1
        // TODO how to make the state is finite?
        while (std::isalnum(state.peek()) || state.peek() == '_') {
            char c = state.advance();
            lexeme += c;

            // LOGGING STATE TRANSITION
            if (first) {
                std::cout << c << " => State " << stateCount << " ";
                first = false;
            }
            else {
                std::cout << c << " => State " << stateCount;
            }

            stateCount++;
        }

        std::string lowerLexeme = lexeme;
        std::transform(lowerLexeme.begin(), lowerLexeme.end(), lowerLexeme.begin(), ::tolower);

        auto it = keywordsMap.find(lowerLexeme);
        if (it != keywordsMap.end()) {
            // LOGGING ACCEPTING STATE (Keyword)
            // Keknya bagian final state lebih bagus dibikin identifier gitu untuk masing-masing token Type
            std::cout << " => State " << "Final State" << " => Gotten: keyword(" << lexeme << ")\n";
            return Token(it->second, lexeme, startLine, startColumn);
        }

        // LOGGING ACCEPTING STATE (Identifier)
        // TODO Ada bug jadi semuanya default to identifier 
        std::cout << "=> State " << "Final State" << " => Gotten: ident(" << lexeme << ")\n";
        return Token(TokenType::IDENT, lexeme, startLine, startColumn);
    }

    // Numeric (Int, Real)
    // TODO : Implement scan numeric
    Token scanNumeric(LexerState &state);

    // Text (Char, String)
    // TODO : Implement scan text
    Token scanText(LexerState &state);

    // Symbols & Comments
    // TODO : Implement scan symbol
    Token scanSymbol(LexerState &state)
    {
        int startLine = state.currentLine;
        int startColumn = state.currentColumn;
        char c = state.advance();
        switch (c) {
            case ';':
                return Token(TokenType::SEMICOLON, ";", startLine, startColumn);

            default:
                return Token(TokenType::ERROR_TOKEN, std::string(1, c), startLine, startColumn);
        }
    }

}
