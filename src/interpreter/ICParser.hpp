#pragma once

#include "../common/Instruction.hpp"

#include <string>
#include <vector>

class ICParser {
public:
    static std::vector<Instruction> parse(const std::string& text);
    static std::vector<Instruction> parseFile(const std::string& path);

private:
    static OpCode parseOpCode(const std::string& value, int lineNumber);
    static RuntimeValue parseLiteral(const std::string& value);
};
