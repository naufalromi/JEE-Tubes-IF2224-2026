#include "SpecificScanners.hpp"
#include <cctype> // Untuk std::isalnum, std::isalpha, std::isdigit
#include <iostream>
#include <algorithm> // for transform

namespace SpecificScanners
{
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
        while (std::isalnum(static_cast<unsigned char>(state.peek())) || state.peek() == '_')
        {
            char c = state.advance();
            lexeme += c;

            // LOGGING STATE TRANSITION
            if (first)
            {
                std::cout << c << " => State " << stateCount << " ";
                first = false;
            }
            else
            {
                std::cout << c << " => State " << stateCount;
            }

            stateCount++;
        }

        std::string lowerLexeme = lexeme;
        std::transform(lowerLexeme.begin(), lowerLexeme.end(), lowerLexeme.begin(), [](unsigned char ch)
                       { return static_cast<char>(std::tolower(ch)); });

        auto it = keywordsMap.find(lowerLexeme);
        if (it != keywordsMap.end())
        {
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
