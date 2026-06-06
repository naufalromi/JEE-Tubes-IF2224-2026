#pragma once

#include "../common/Instruction.hpp"

#include <ostream>
#include <string>
#include <vector>

class Interpreter {
public:
    static constexpr int STACK_MAX = 1000000;

    Interpreter();
    explicit Interpreter(std::vector<Instruction> instructions);

    void setInstructions(std::vector<Instruction> instructions);
    void run(std::ostream& out);

    const std::string& getOutput() const;
    const std::vector<Instruction>& getInstructions() const;

    static void printInstructions(const std::vector<Instruction>& instructions, std::ostream& out);

private:
    std::vector<Instruction> instructions;
    std::vector<RuntimeValue> stack;
    int sp;
    int bp;
    int pc;
    bool halted;
    std::string output;

    void reset();
    void execute(const Instruction& instr, std::ostream& out);
    void executeOpr(int op, int line, std::ostream& out);

    int base(int level) const;
    void validateJumpTarget(int target, int line) const;
    void ensureStackIndex(int index, int line) const;
    void requireStackItems(int count, int line) const;

    RuntimeValue pop(int line);
    void push(const RuntimeValue& value, int line);

    static int toInt(const RuntimeValue& value, int line);
    static double toDouble(const RuntimeValue& value, int line);
    static bool isNumeric(const RuntimeValue& value);
    static bool isDoubleValue(const RuntimeValue& value);
    static bool isEqual(const RuntimeValue& left, const RuntimeValue& right);
    static int compare(const RuntimeValue& left, const RuntimeValue& right, int line);
};
