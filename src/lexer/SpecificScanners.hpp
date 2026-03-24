#pragma once
#include "../common/Token.hpp"
#include "LexerState.hpp"
#include <unordered_map>
#include <string>

namespace SpecificScanners{
    // Alphanumeric (Identifier, Keyword)
    Token scanAlpha(LexerState& state, const std::unordered_map<std::string, TokenType>& keywordsMap);
    
    // Numeric (Int, Real)
    Token scanNumeric(LexerState& state);
    
    // Text (Char, String)
    Token scanText(LexerState& state);
    
    // Symbols & Comments
    Token scanSymbol(LexerState& state);

}