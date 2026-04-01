#pragma once
#include "../common/Token.hpp"
#include "LexerState.hpp"
#include <unordered_map>
#include <string>
#include <vector>

namespace SpecificScanners{
    // Alphanumeric (Identifier, Keyword)
    Token scanAlpha(LexerState &state, const std::vector<std::string> &keywordsArray, const std::vector<TokenType> &tokenTypeArray);
    
    // Numeric (Int, Real)
    Token scanNumeric(LexerState& state);
    
    // Text (Char, String)
    Token scanText(LexerState& state);
    
    // Symbols & Comments
    Token scanSymbol(LexerState& state);

}