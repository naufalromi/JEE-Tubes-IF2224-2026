#include "Instruction.hpp"

#include <sstream>

std::string opCodeToString(OpCode op)
{
    switch (op) {
        case OpCode::LIT: return "LIT";
        case OpCode::LOD: return "LOD";
        case OpCode::STO: return "STO";
        case OpCode::CAL: return "CAL";
        case OpCode::INT: return "INT";
        case OpCode::JMP: return "JMP";
        case OpCode::JPC: return "JPC";
        case OpCode::OPR: return "OPR";
        case OpCode::RET: return "RET";
        case OpCode::LDA: return "LDA";
        case OpCode::LDI: return "LDI";
        case OpCode::STI: return "STI";
        case OpCode::STA: return "STA";
        default: return "UNKNOWN";
    }
}

std::string instructionToString(int index, const Instruction& instr)
{
    std::ostringstream oss;

    oss << index << " " << opCodeToString(instr.op);

    if (instr.op == OpCode::RET) {
        return oss.str();
    }

    oss << " " << instr.level << " ";

    if (instr.op == OpCode::LIT && instr.hasLiteral) {
        oss << runtimeValueToString(instr.literal);
    } else {
        oss << instr.operand;
    }

    return oss.str();
}
