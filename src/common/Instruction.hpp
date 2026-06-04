#pragma once

#include <string>

#include "RuntimeValue.hpp"

enum class OpCode {
    LIT,
    LOD,
    STO,
    CAL,
    INT,
    JMP,
    JPC,
    OPR,
    RET,
    LDA,
    LDI,
    STI 
};

enum class OperatorCode {
    NEG   = 1,
    ADD   = 2,
    SUB   = 3,
    MUL   = 4,
    DIV   = 5,
    MOD   = 6,
    EQL   = 7,
    NEQ   = 8,
    LSS   = 9,
    GEQ   = 10,
    GTR   = 11,
    LEQ   = 12,
    WRT   = 13,
    WRTLN = 14
};

struct Instruction {
    int line;
    OpCode op;
    int level;
    int operand;

    bool hasLiteral;
    RuntimeValue literal;

    Instruction(OpCode op, int level, int operand)
        : line(-1),
          op(op),
          level(level),
          operand(operand),
          hasLiteral(false),
          literal(0) {}

    Instruction(OpCode op, int level, RuntimeValue literal)
        : line(-1),
          op(op),
          level(level),
          operand(0),
          hasLiteral(true),
          literal(literal) {}

    Instruction(int line, OpCode op, int level, int operand)
        : line(line),
          op(op),
          level(level),
          operand(operand),
          hasLiteral(false),
          literal(0) {}

    Instruction(int line, OpCode op, int level, RuntimeValue literal)
        : line(line),
          op(op),
          level(level),
          operand(0),
          hasLiteral(true),
          literal(literal) {}
};

std::string opCodeToString(OpCode op);

std::string instructionToString(int index, const Instruction& instr);
