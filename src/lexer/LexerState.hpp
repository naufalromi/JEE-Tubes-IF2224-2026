#pragma once
#include <string>

/**
 * Struct untuk spesifict DFA
 */
struct LexerState {
    const std::string &source;
    size_t currentIndex;
    int currentLine;
    int currentColumn;

    LexerState(const std::string &src) : source(src), currentIndex(0), currentLine(1), currentColumn(1) {}

    bool isAtEnd() const
    {
        return currentIndex >= source.length();
    }

    char peek() const
    {
        if (isAtEnd()) {
            return '\0';
        }
        else {
            return source[currentIndex];
        }
    }

    char peekNext() const
    {
        if (currentIndex + 1 >= source.length()) {
            return '\0';
        }
        return source[currentIndex + 1];
    }

    char advance()
    {
        if (isAtEnd()) {
            return '\0';
        }
        char currentChar = source[currentIndex];
        currentIndex++;
        if (currentChar == '\n') {
            currentLine++;
            currentColumn = 1; // reset ke kolom 1 jika pindah baris
        }
        else {
            currentColumn++;
        }
        return currentChar;
    }
};
