#include "SpecificScanners.hpp"
#include <cctype> // Untuk std::isalnum, std::isalpha, std::isdigit
#include <iostream>
#include <algorithm> // for transform


namespace SpecificScanners
{
    /**
     * @brief Given an array of keywords, checks how many of them are matching based on the current lexeme
     * and returns all the matching keywords on a new string array
     * 
     * @param keywordsArray Keywords that are matched so far
     * @param lexeme The current lexeme
     * @return an array with the format of [keyword1, keyword1idx, keyword2, keyword2idx, ..., keywordn, keywordnidx] 
     * The idx is the original idx from the original keywords array (to track states)
     */
    static std::vector<std::string> filterKeywords(const std::vector<std::string> &keywordsArray, std::string &lexeme){
        std::vector<std::string> filteredKeywords;
        int newCharIdx = lexeme.length()-1;
        bool firstFilter = (lexeme.length()==1);

        int incr = firstFilter?1:2;
        for(size_t i = 0; i<keywordsArray.size(); i+=incr){
            if(keywordsArray[i].length()>=(lexeme.length()) && std::tolower(static_cast<unsigned char>(keywordsArray[i][newCharIdx])) == std::tolower(static_cast<unsigned char>(lexeme[newCharIdx])) ){
                filteredKeywords.push_back(keywordsArray[i]);
                if(firstFilter){
                    //This means that the keywordsArray is still the original keywordsArray and not
                    //in the format of [keyword1, keyword1idx...]
                    filteredKeywords.push_back(std::to_string(i));
                }
                else{
                    //This means the keywordsArray is already in the format where the idx is placed
                    //right after the keyword. Thus this push back preserves the original idx for consistency
                    filteredKeywords.push_back(keywordsArray[i+1]);
                }
            }
            // else it wont get added to the filtered keywords array
        }

        return filteredKeywords;
    }
    /**
     * @brief Get the state based on the filtered array
     * @param filteredKeywords 
     * @return returns the state in the form of a string with the format idx1_idx2..._idxn
     */
    static std::string getState(std::vector<std::string> &filteredKeywords){
        std::string state;
        for(size_t i = 1; i<filteredKeywords.size(); i+=2){
            if(i != 1){
                state += "_";
            }
            state += filteredKeywords[i];
        }
        return state;
    }

    // Alphanumeric (Identifier, Keyword)
    Token scanAlpha(LexerState &state, const std::vector<std::string> &keywordsArray, const std::vector<TokenType> &tokenTypeArray)
    {
        /**
         * State guides
         * Format State <state code>
         * 
         * State codes meaning:
         * a<idx1>_<idx2>_<idx3>...._<idxn>_i<length of lexeme>:
         * Means that this is a state where there are n keywords matched, with the keywords index in the
         * original keywords array written. The order of the keywords are alphabetical. The last number is
         * i<length of lexeme> which is used to avoid ambiguity between states. Without this, for example
         * lets say the keyword abc is in index 1, and abcd is in index 2, the state "State a1_2" can 
         * be ambiguous because the lexeme "a", "ab", and "abc" returns the same state.
         * 
         * aS:
         * Means that this is the start state of the Alpha scanner
         * 
         * a<size of the original keywordsArray>:
         * Means that it is definitely an ident and not a keyword, but the lexeme is not yet complete (belum ketemu spasi)
         */


        int startLine = state.currentLine;
        int startColumn = state.currentColumn;
        bool first = true;
        std::string lexeme = "";
        std::vector<std::string> filteredKeywords;
        
        // Looping ini adalah State 1
        while (std::isalnum(static_cast<unsigned char>(state.peek())) || state.peek() == '_')
        {
            char c = state.advance();
            lexeme += c;

            if (first) {
                filteredKeywords = filterKeywords(keywordsArray, lexeme);
            } else {
                filteredKeywords = filterKeywords(filteredKeywords, lexeme);
            }

            // LOGGING STATE TRANSITION
            if (first){
                std::cout << std::endl << c << " => State aS\n";
                first = false;
            }
            else if(filteredKeywords.size()==0){
                //State (number of keywords) represents the state in which there is no keywords matching left, thus
                //signifying tis an ident
                //e.g "a27" if the total number of keywords are 27
                std::cout << c << " => State a" << std::to_string(keywordsArray.size()) << std::endl;
            }
            else if(filteredKeywords.size()>=2){
                //Means there is at least one pair of [keyword, idx] but the lexeme scanning isnt yet done
                //a<idx1>_<idx2>_<idx3>...._<idxn>_i<length of lexeme>
                std::cout << c << " => State a" << getState(filteredKeywords) << "_i"<< std::to_string(lexeme.length()) <<std::endl;
            }
        }

        std::string lowerLexeme = lexeme;
        std::transform(lowerLexeme.begin(), lowerLexeme.end(), lowerLexeme.begin(), [](unsigned char ch)
                       { return static_cast<char>(std::tolower(ch)); });

        /*
        Because the keywords are ordered alphabetically, if there is more than 1 keyword matching in the final
        filtered keywords, it means it will more closely match the keyword on the smaller index
        */
        bool isIdent;
        if(filteredKeywords.size()==0){
            isIdent = true;
        }
        else{
            if(filteredKeywords[0].length()!=lexeme.length()) isIdent = true;
            else isIdent = false;
        }

        if(isIdent){
            // LOGGING ACCEPTING STATE (Identifier)
            std::cout << "=> State " << "Final State" << " => Gotten: ident(" << lexeme << ")\n";
            return Token(TokenType::IDENT, lexeme, startLine, startColumn);
        }
        else
        {
            // LOGGING ACCEPTING STATE (Keyword)
            std::cout << "=> State " << "Final State" << " => Gotten: keyword(" << lexeme << ")\n";
            TokenType t = tokenTypeArray[std::stoi(filteredKeywords[1])];
            return Token(t, lexeme, startLine, startColumn);
        }

    }

    // Numeric (Int, Real)
    // TODO : Implement scan numeric
    Token scanNumeric(LexerState &state)
    {
        int startLine = state.currentLine;
        int startColumn = state.currentColumn;
        std::string lexeme = "";
        bool isReal = false;

        // state on number
        while (std::isdigit(static_cast<unsigned char>(state.peek())))
        {
            lexeme += state.advance();
        }

        // if meet dot become real
        if (state.peek() == '.')
        {

            // incomplete real 
            if (!std::isdigit(static_cast<unsigned char>(state.peekNext())))
            {
                lexeme += state.advance(); // consume '.'
                return Token(TokenType::ERROR_TOKEN, ErrorType::IncompleteReal, startLine, startColumn, lexeme);
            }

            isReal = true;
            lexeme += state.advance();

            // read num again
            while (std::isdigit(static_cast<unsigned char>(state.peek())))
            {
                lexeme += state.advance();
            }
        }

        // exponent cuz its cool
        if (state.peek() == 'e' || state.peek() == 'E')
        {
            size_t backupIndex = state.currentIndex;
            int backupLine = state.currentLine;
            int backupColumn = state.currentColumn;

            std::string exponentPart = "";
            exponentPart += state.advance(); // e or E

            if (state.peek() == '+' || state.peek() == '-')
            {
                exponentPart += state.advance();
            }

            if (std::isdigit(static_cast<unsigned char>(state.peek())))
            {
                isReal = true;
                while (std::isdigit(static_cast<unsigned char>(state.peek())))
                {
                    exponentPart += state.advance();
                }
                lexeme += exponentPart;
            }
            else
            {
                // fallback if exponent not valid
                state.currentIndex = backupIndex;
                state.currentLine = backupLine;
                state.currentColumn = backupColumn;
            }
        }

        return Token(isReal ? TokenType::REALCON : TokenType::INTCON, lexeme, startLine, startColumn);
    }

    // Text (Char, String)
    // TODO : Implement scan text
    Token scanText(LexerState &state)
    {
        int startLine = state.currentLine;
        int startColumn = state.currentColumn;
        std::string lexeme = "";

        if (state.advance() != '\'')
        {
            return Token(TokenType::ERROR_TOKEN, ErrorType::UnterminatedString, startLine, startColumn);
        }

        while (!state.isAtEnd())
        {
            char c = state.advance();

            if (c == '\'')
            {
                // single quote ''
                if (state.peek() == '\'')
                {
                    state.advance();
                    lexeme += '\'';
                    continue;
                }

                if (lexeme.length() == 1)
                {
                    return Token(TokenType::CHARCON, lexeme, startLine, startColumn);
                }
                return Token(TokenType::STRINGCON, lexeme, startLine, startColumn);
            }

            if (c == '\n' || c == '\0')
            {
                return Token(TokenType::ERROR_TOKEN, ErrorType::UnterminatedString, startLine, startColumn, lexeme);
            }

            lexeme += c;
        }

        return Token(TokenType::ERROR_TOKEN, ErrorType::UnterminatedString, startLine, startColumn, lexeme);
    }

    // Symbols & Comments
    // TODO : Implement scan symbol

    // self... explanatory
    Token scanSymbol(LexerState &state)
    {
        int startLine = state.currentLine;
        int startColumn = state.currentColumn;
        char c = state.advance();

        switch (c)
        {
        case ';':
            return Token(TokenType::SEMICOLON, ";", startLine, startColumn);
        case ',':
            return Token(TokenType::COMMA, ",", startLine, startColumn);
        case '.':
            return Token(TokenType::PERIOD, ".", startLine, startColumn);
        case '+':
            return Token(TokenType::PLUS, "+", startLine, startColumn);
        case '-':
            return Token(TokenType::MINUS, "-", startLine, startColumn);
        case '*':
            return Token(TokenType::TIMES, "*", startLine, startColumn);
        case '/':
            return Token(TokenType::RDIV, "/", startLine, startColumn);

        // comment v1
        case '(':
            if (state.peek() == '*')
            {
                state.advance(); // consume '*'
                while (!state.isAtEnd())
                {
                    char cc = state.advance();
                    if (cc == '*' && state.peek() == ')')
                    {
                        state.advance(); // consume ')'
                        return Token(TokenType::COMMENT, "(**)", startLine, startColumn);
                    }
                }
                return Token(TokenType::ERROR_TOKEN, ErrorType::UnterminatedComment, startLine, startColumn);
            }
            return Token(TokenType::LPARENT, "(", startLine, startColumn);

        case ')':
            return Token(TokenType::RPARENT, ")", startLine, startColumn);
        case '[':
            return Token(TokenType::LBRACK, "[", startLine, startColumn);
        case ']':
            return Token(TokenType::RBRACK, "]", startLine, startColumn);

        case ':':
            if (state.peek() == '=')
            {
                state.advance();
                return Token(TokenType::BECOMES, ":=", startLine, startColumn);
            }
            return Token(TokenType::COLON, ":", startLine, startColumn);

        case '<':
            if (state.peek() == '=')
            {
                state.advance();
                return Token(TokenType::LEQ, "<=", startLine, startColumn);
            }
            if (state.peek() == '>')
            {
                state.advance();
                return Token(TokenType::NEQ, "<>", startLine, startColumn);
            }
            return Token(TokenType::LSS, "<", startLine, startColumn);

        case '>':
            if (state.peek() == '=')
            {
                state.advance();
                return Token(TokenType::GEQ, ">=", startLine, startColumn);
            }
            return Token(TokenType::GTR, ">", startLine, startColumn);

        case '=':
            return Token(TokenType::EQL, "=", startLine, startColumn);

        // comment v2
        case '{':
        {
            while (!state.isAtEnd() && state.peek() != '}')
            {
                state.advance();
            }
            if (state.isAtEnd())
            {
                return Token(TokenType::ERROR_TOKEN, ErrorType::UnterminatedComment, startLine, startColumn);
            }
            state.advance(); // consume '}'
            return Token(TokenType::COMMENT, "{}", startLine, startColumn);
        }

        default:
            return Token(TokenType::ERROR_TOKEN, ErrorType::IllegalChar, startLine, startColumn, std::string(1, c));
        }
    }

}
