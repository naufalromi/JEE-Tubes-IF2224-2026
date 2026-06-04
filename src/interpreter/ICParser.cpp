#include "ICParser.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
std::string trim(const std::string& value)
{
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        start++;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        end--;
    }

    return value.substr(start, end - start);
}

bool isIntegerLiteral(const std::string& value)
{
    if (value.empty()) {
        return false;
    }

    size_t index = (value[0] == '-' || value[0] == '+') ? 1 : 0;
    if (index == value.size()) {
        return false;
    }

    for (; index < value.size(); index++) {
        if (!std::isdigit(static_cast<unsigned char>(value[index]))) {
            return false;
        }
    }

    return true;
}

bool isDoubleLiteral(const std::string& value)
{
    bool hasDigit = false;
    bool hasDot = false;
    size_t index = (value.empty() || (value[0] != '-' && value[0] != '+')) ? 0 : 1;

    for (; index < value.size(); index++) {
        char ch = value[index];
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            hasDigit = true;
        } else if (ch == '.' && !hasDot) {
            hasDot = true;
        } else {
            return false;
        }
    }

    return hasDigit && hasDot;
}

std::string unquote(const std::string& value)
{
    if (value.size() < 2) {
        return value;
    }

    char quote = value.front();
    if ((quote != '"' && quote != '\'') || value.back() != quote) {
        return value;
    }

    std::string result;
    for (size_t index = 1; index + 1 < value.size(); index++) {
        if (value[index] == '\\' && index + 1 < value.size() - 1) {
            index++;
            switch (value[index]) {
                case 'n': result.push_back('\n'); break;
                case 't': result.push_back('\t'); break;
                case 'r': result.push_back('\r'); break;
                case '\\': result.push_back('\\'); break;
                case '\'': result.push_back('\''); break;
                case '"': result.push_back('"'); break;
                default: result.push_back(value[index]); break;
            }
        } else {
            result.push_back(value[index]);
        }
    }

    return result;
}
}

std::vector<Instruction> ICParser::parse(const std::string& text)
{
    std::vector<Instruction> instructions;
    std::istringstream input(text);
    std::string rawLine;
    int physicalLine = 0;

    while (std::getline(input, rawLine)) {
        physicalLine++;
        std::string line = trim(rawLine);
        if (line.empty()) {
            continue;
        }

        std::istringstream lineInput(line);
        int lineNumber;
        std::string opText;

        if (!(lineInput >> lineNumber >> opText)) {
            throw std::runtime_error("Invalid IC at input line " + std::to_string(physicalLine));
        }

        if (lineNumber != static_cast<int>(instructions.size())) {
            throw std::runtime_error(
                "Invalid IC line number " + std::to_string(lineNumber) +
                ", expected " + std::to_string(instructions.size())
            );
        }

        OpCode op = parseOpCode(opText, lineNumber);

        if (op == OpCode::RET) {
            int level = 0;
            int operand = 0;
            if (lineInput >> level) {
                if (!(lineInput >> operand)) {
                    throw std::runtime_error("RET must have either no operands or both level and operand at IC line " + std::to_string(lineNumber));
                }
                if (level != 0 || operand != 0) {
                    throw std::runtime_error("RET operands must be 0 0 at IC line " + std::to_string(lineNumber));
                }
                std::string extra;
                if (lineInput >> extra) {
                    throw std::runtime_error("Unexpected token at IC line " + std::to_string(lineNumber) + ": " + extra);
                }
            }

            instructions.emplace_back(lineNumber, op, 0, 0);
            continue;
        }

        int level;
        if (!(lineInput >> level)) {
            throw std::runtime_error("Missing level at IC line " + std::to_string(lineNumber));
        }

        if (op == OpCode::LIT) {
            std::string literalText;
            if (!(lineInput >> literalText)) {
                throw std::runtime_error("Missing literal operand at IC line " + std::to_string(lineNumber));
            }

            std::string rest;
            std::getline(lineInput, rest);
            rest = trim(rest);
            if (!rest.empty()) {
                literalText += " " + rest;
            }

            instructions.emplace_back(lineNumber, op, level, parseLiteral(literalText));
        } else {
            int operand;
            if (!(lineInput >> operand)) {
                throw std::runtime_error("Missing operand at IC line " + std::to_string(lineNumber));
            }

            std::string extra;
            if (lineInput >> extra) {
                throw std::runtime_error("Unexpected token at IC line " + std::to_string(lineNumber) + ": " + extra);
            }

            instructions.emplace_back(lineNumber, op, level, operand);
        }
    }

    return instructions;
}

std::vector<Instruction> ICParser::parseFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open IC file: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return parse(buffer.str());
}

OpCode ICParser::parseOpCode(const std::string& value, int lineNumber)
{
    if (value == "LIT") return OpCode::LIT;
    if (value == "LOD") return OpCode::LOD;
    if (value == "STO") return OpCode::STO;
    if (value == "CAL") return OpCode::CAL;
    if (value == "INT") return OpCode::INT;
    if (value == "JMP") return OpCode::JMP;
    if (value == "JPC") return OpCode::JPC;
    if (value == "OPR") return OpCode::OPR;
    if (value == "RET") return OpCode::RET;

    throw std::runtime_error("Unknown opcode at IC line " + std::to_string(lineNumber) + ": " + value);
}

RuntimeValue ICParser::parseLiteral(const std::string& value)
{
    std::string literal = trim(value);

    if (literal == "true") {
        return true;
    }
    if (literal == "false") {
        return false;
    }
    if (isIntegerLiteral(literal)) {
        return std::stoi(literal);
    }
    if (isDoubleLiteral(literal)) {
        return std::stod(literal);
    }
    if (literal.size() >= 2 && literal.front() == '\'' && literal.back() == '\'') {
        std::string text = unquote(literal);
        if (text.size() != 1) {
            throw std::runtime_error("Character literal must contain exactly one character: " + literal);
        }
        return text[0];
    }
    if (literal.size() >= 2 && literal.front() == '"' && literal.back() == '"') {
        return unquote(literal);
    }

    return literal;
}
