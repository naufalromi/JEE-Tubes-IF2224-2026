#include "Interpreter.hpp"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace {
int lineOf(const Instruction& instr)
{
    return instr.line >= 0 ? instr.line : -1;
}

std::string runtimeErrorPrefix(int line)
{
    if (line >= 0) {
        return "RuntimeError at line " + std::to_string(line) + ": ";
    }
    return "RuntimeError: ";
}

RuntimeValue numericResult(double value, bool forceDouble)
{
    if (!forceDouble && std::floor(value) == value) {
        return static_cast<int>(value);
    }
    return value;
}
}

Interpreter::Interpreter()
    : stack(STACK_MAX, 0),
      sp(-1),
      bp(0),
      pc(0),
      halted(false)
{
}

Interpreter::Interpreter(std::vector<Instruction> instructions)
    : Interpreter()
{
    setInstructions(std::move(instructions));
}

void Interpreter::setInstructions(std::vector<Instruction> instructions)
{
    this->instructions = std::move(instructions);
}

void Interpreter::run(std::ostream& out)
{
    reset();

    while (!halted) {
        if (pc < 0 || pc >= static_cast<int>(instructions.size())) {
            throw std::runtime_error(runtimeErrorPrefix(pc) + "Program counter out of range");
        }

        const Instruction instr = instructions[pc];
        pc++;
        execute(instr, out);
    }
}

const std::string& Interpreter::getOutput() const
{
    return output;
}

const std::vector<Instruction>& Interpreter::getInstructions() const
{
    return instructions;
}

void Interpreter::printInstructions(const std::vector<Instruction>& instructions, std::ostream& out)
{
    for (int index = 0; index < static_cast<int>(instructions.size()); index++) {
        out << instructionToString(index, instructions[index]) << '\n';
    }
}

void Interpreter::reset()
{
    stack.assign(STACK_MAX, 0);
    sp = -1;
    bp = 0;
    pc = 0;
    halted = false;
    output.clear();

    stack[0] = 0;
    stack[1] = 0;
    stack[2] = 0;
}

void Interpreter::execute(const Instruction& instr, std::ostream& out)
{
    int line = lineOf(instr);

    switch (instr.op) {
        case OpCode::INT:
            sp += instr.operand;
            ensureStackIndex(sp, line);
            break;

        case OpCode::LIT:
            push(instr.hasLiteral ? instr.literal : RuntimeValue(instr.operand), line);
            break;

        case OpCode::LOD: {
            int address = base(instr.level) + instr.operand;
            ensureStackIndex(address, line);
            push(stack[address], line);
            break;
        }

        case OpCode::STO: {
            RuntimeValue value = pop(line);
            int address = base(instr.level) + instr.operand;
            ensureStackIndex(address, line);
            stack[address] = value;
            break;
        }

        case OpCode::CAL:
            validateJumpTarget(instr.operand, line);
            ensureStackIndex(sp + 3, line);
            stack[sp + 1] = base(instr.level);
            stack[sp + 2] = bp;
            stack[sp + 3] = pc;
            bp = sp + 1;
            pc = instr.operand;
            break;

        case OpCode::JMP:
            validateJumpTarget(instr.operand, line);
            pc = instr.operand;
            break;

        case OpCode::JPC: {
            RuntimeValue value = pop(line);
            if (toInt(value, line) == 0) {
                validateJumpTarget(instr.operand, line);
                pc = instr.operand;
            }
            break;
        }

        case OpCode::OPR:
            executeOpr(instr.operand, line, out);
            break;

        case OpCode::RET:
            ensureStackIndex(bp + 2, line);
            sp = bp - 1;
            pc = toInt(stack[bp + 2], line);
            bp = toInt(stack[bp + 1], line);
            if (pc == 0) {
                halted = true;
            }
            break;
    }
}

void Interpreter::executeOpr(int op, int line, std::ostream& out)
{
    switch (static_cast<OperatorCode>(op)) {
        case OperatorCode::NEG: {
            requireStackItems(1, line);
            bool forceDouble = isDoubleValue(stack[sp]);
            stack[sp] = numericResult(-toDouble(stack[sp], line), forceDouble);
            break;
        }

        case OperatorCode::ADD: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            bool forceDouble = isDoubleValue(left) || isDoubleValue(right);
            push(numericResult(toDouble(left, line) + toDouble(right, line), forceDouble), line);
            break;
        }

        case OperatorCode::SUB: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            bool forceDouble = isDoubleValue(left) || isDoubleValue(right);
            push(numericResult(toDouble(left, line) - toDouble(right, line), forceDouble), line);
            break;
        }

        case OperatorCode::MUL: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            bool forceDouble = isDoubleValue(left) || isDoubleValue(right);
            push(numericResult(toDouble(left, line) * toDouble(right, line), forceDouble), line);
            break;
        }

        case OperatorCode::DIV: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            int divisor = toInt(right, line);
            if (divisor == 0) {
                throw std::runtime_error(runtimeErrorPrefix(line) + "Division by zero");
            }
            push(toInt(left, line) / divisor, line);
            break;
        }

        case OperatorCode::MOD: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            int divisor = toInt(right, line);
            if (divisor == 0) {
                throw std::runtime_error(runtimeErrorPrefix(line) + "Modulo by zero");
            }
            push(toInt(left, line) % divisor, line);
            break;
        }

        case OperatorCode::EQL: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            push(isEqual(left, right) ? 1 : 0, line);
            break;
        }

        case OperatorCode::NEQ: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            push(isEqual(left, right) ? 0 : 1, line);
            break;
        }

        case OperatorCode::LSS: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            push(compare(left, right, line) < 0 ? 1 : 0, line);
            break;
        }

        case OperatorCode::GEQ: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            push(compare(left, right, line) >= 0 ? 1 : 0, line);
            break;
        }

        case OperatorCode::GTR: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            push(compare(left, right, line) > 0 ? 1 : 0, line);
            break;
        }

        case OperatorCode::LEQ: {
            RuntimeValue right = pop(line);
            RuntimeValue left = pop(line);
            push(compare(left, right, line) <= 0 ? 1 : 0, line);
            break;
        }

        case OperatorCode::WRT: {
            RuntimeValue value = pop(line);
            std::string text = runtimeValueToString(value);
            output += text;
            out << text;
            break;
        }

        case OperatorCode::WRTLN: {
            RuntimeValue value = pop(line);
            std::string text = runtimeValueToString(value);
            output += text + "\n";
            out << text << '\n';
            break;
        }

        default:
            throw std::runtime_error(runtimeErrorPrefix(line) + "Unknown OPR code " + std::to_string(op));
    }
}

int Interpreter::base(int level) const
{
    int currentBase = bp;
    while (level > 0) {
        if (currentBase < 0 || currentBase >= STACK_MAX) {
            throw std::runtime_error("RuntimeError: Static link base out of range");
        }
        currentBase = toInt(stack[currentBase], -1);
        level--;
    }
    return currentBase;
}

void Interpreter::validateJumpTarget(int target, int line) const
{
    if (target < 0 || target >= static_cast<int>(instructions.size())) {
        throw std::runtime_error(runtimeErrorPrefix(line) + "Invalid jump target " + std::to_string(target));
    }
}

void Interpreter::ensureStackIndex(int index, int line) const
{
    if (index < 0) {
        throw std::runtime_error(runtimeErrorPrefix(line) + "Stack underflow");
    }
    if (index >= STACK_MAX) {
        throw std::runtime_error(runtimeErrorPrefix(line) + "Stack overflow");
    }
}

void Interpreter::requireStackItems(int count, int line) const
{
    if (sp - count + 1 < 0) {
        throw std::runtime_error(runtimeErrorPrefix(line) + "Stack underflow");
    }
}

RuntimeValue Interpreter::pop(int line)
{
    requireStackItems(1, line);
    RuntimeValue value = stack[sp];
    sp--;
    return value;
}

void Interpreter::push(const RuntimeValue& value, int line)
{
    sp++;
    ensureStackIndex(sp, line);
    stack[sp] = value;
}

int Interpreter::toInt(const RuntimeValue& value, int line)
{
    return std::visit([line](const auto& raw) -> int {
        using T = std::decay_t<decltype(raw)>;

        if constexpr (std::is_same_v<T, int>) {
            return raw;
        } else if constexpr (std::is_same_v<T, double>) {
            return static_cast<int>(raw);
        } else if constexpr (std::is_same_v<T, bool>) {
            return raw ? 1 : 0;
        } else if constexpr (std::is_same_v<T, char>) {
            return static_cast<int>(raw);
        } else {
            try {
                return std::stoi(raw);
            } catch (...) {
                throw std::runtime_error(runtimeErrorPrefix(line) + "Expected integer-compatible value");
            }
        }
    }, value);
}

double Interpreter::toDouble(const RuntimeValue& value, int line)
{
    return std::visit([line](const auto& raw) -> double {
        using T = std::decay_t<decltype(raw)>;

        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
            return raw;
        } else if constexpr (std::is_same_v<T, bool>) {
            return raw ? 1.0 : 0.0;
        } else if constexpr (std::is_same_v<T, char>) {
            return static_cast<double>(raw);
        } else {
            try {
                return std::stod(raw);
            } catch (...) {
                throw std::runtime_error(runtimeErrorPrefix(line) + "Expected numeric value");
            }
        }
    }, value);
}

bool Interpreter::isNumeric(const RuntimeValue& value)
{
    return std::holds_alternative<int>(value) ||
           std::holds_alternative<double>(value) ||
           std::holds_alternative<bool>(value) ||
           std::holds_alternative<char>(value);
}

bool Interpreter::isDoubleValue(const RuntimeValue& value)
{
    return std::holds_alternative<double>(value);
}

bool Interpreter::isEqual(const RuntimeValue& left, const RuntimeValue& right)
{
    if (isNumeric(left) && isNumeric(right)) {
        return toDouble(left, -1) == toDouble(right, -1);
    }
    return runtimeValueToString(left) == runtimeValueToString(right);
}

int Interpreter::compare(const RuntimeValue& left, const RuntimeValue& right, int line)
{
    if (isNumeric(left) && isNumeric(right)) {
        double leftValue = toDouble(left, line);
        double rightValue = toDouble(right, line);
        if (leftValue < rightValue) return -1;
        if (leftValue > rightValue) return 1;
        return 0;
    }

    std::string leftValue = runtimeValueToString(left);
    std::string rightValue = runtimeValueToString(right);
    if (leftValue < rightValue) return -1;
    if (leftValue > rightValue) return 1;
    return 0;
}
