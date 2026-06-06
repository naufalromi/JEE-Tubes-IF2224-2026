#include "CodeGenerator.hpp"
#include "../semantic/ASTNode.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

CodeGenerator::CodeGenerator(const SymbolTable *symTab) : symbolTable(symTab) {}

void CodeGenerator::generate(ProgramNode *root)
{
    instructions.clear();
    subroutineAddresses.clear();
    unresolvedCalls.clear();
    currentLevel = 0;

    if (root) {
        root->accept(this);
    }

    resolveFunctionCalls();
}

const std::vector<Instruction> &CodeGenerator::getInstructions() const
{
    return instructions;
}

void CodeGenerator::emit(OpCode op, int level, int operand)
{
    instructions.push_back(Instruction(op, level, operand));
}

void CodeGenerator::emitLiteral(int level, RuntimeValue value)
{
    instructions.push_back(Instruction(OpCode::LIT, level, value));
}

bool CodeGenerator::isCompositeType(DataType type) const
{
    return type == DataType::ARRAY || type == DataType::RECORD;
}

void CodeGenerator::emitAddress(ExpressionNode *node)
{
    if (!node) return;

    if (auto varNode = dynamic_cast<VarAccessNode *>(node)) {
        int index = varNode->tabIndex;
        int diffLevel = currentLevel - symbolTable->tab[index].lev;

        if (symbolTable->tab[index].nrm == 0) {
            emit(OpCode::LOD, diffLevel, symbolTable->tab[index].adr);
        } else {
            emit(OpCode::LDA, diffLevel, symbolTable->tab[index].adr);
        }
        return;
    }

    if (auto arrayNode = dynamic_cast<ArrayAccessNode *>(node)) {
        emitAddress(arrayNode->target.get());

        int atabIndex = arrayNode->target->evaluatedRef;
        int low = symbolTable->atab[atabIndex].low;
        int elsz = symbolTable->atab[atabIndex].elsz;

        emitValue(arrayNode->index.get());

        if (low != 0) {
            emitLiteral(0, low);
            emit(OpCode::OPR, 0, 3);
        }

        emitLiteral(0, elsz);
        emit(OpCode::OPR, 0, 4);
        emit(OpCode::OPR, 0, 2);
        return;
    }

    if (auto fieldNode = dynamic_cast<FieldAccessNode *>(node)) {
        emitAddress(fieldNode->target.get());
        emitLiteral(0, symbolTable->tab[fieldNode->tabIndex].adr);
        emit(OpCode::OPR, 0, 2);
        return;
    }

    node->accept(this);
}

void CodeGenerator::emitValue(ExpressionNode *node)
{
    if (!node) return;

    if (auto varNode = dynamic_cast<VarAccessNode *>(node)) {
        int index = varNode->tabIndex;
        if (symbolTable->tab[index].obj == ObjectType::CONSTANT) {
            emitLiteral(0, symbolTable->tab[index].adr);
            return;
        }

        emitAddress(node);
        if (!isCompositeType(varNode->evaluatedType)) {
            emit(OpCode::LDI, 0, 0);
        }
        return;
    }

    if (auto arrayNode = dynamic_cast<ArrayAccessNode *>(node)) {
        emitAddress(node);
        if (!isCompositeType(arrayNode->evaluatedType)) {
            emit(OpCode::LDI, 0, 0);
        }
        return;
    }

    if (auto fieldNode = dynamic_cast<FieldAccessNode *>(node)) {
        emitAddress(node);
        if (!isCompositeType(fieldNode->evaluatedType)) {
            emit(OpCode::LDI, 0, 0);
        }
        return;
    }

    node->accept(this);
}

int CodeGenerator::getNextAddress() const
{
    return instructions.size();
}

void CodeGenerator::backpatch(int instructionIndex, int targetAddress)
{
    if (instructionIndex >= 0 && instructionIndex < (int)instructions.size()) {
        instructions[instructionIndex].operand = targetAddress;
    }
}

void CodeGenerator::resolveFunctionCalls()
{
    for (auto &call : unresolvedCalls) {
        int instrIndex = call.first;
        int targetTabIndex = call.second;

        if (subroutineAddresses.find(targetTabIndex) != subroutineAddresses.end()) {
            instructions[instrIndex].operand = subroutineAddresses[targetTabIndex];
        }
        else {
            std::cerr << "Warning: Unresolved subroutine call to tabIndex " << targetTabIndex << "\n";
        }
    }
}

void CodeGenerator::visit(SimpleTypeNode *node) {}
void CodeGenerator::visit(ArrayTypeNode *node) {}
void CodeGenerator::visit(RecordTypeNode *node) {}
void CodeGenerator::visit(EnumeratedTypeNode *node) {}
void CodeGenerator::visit(RangeTypeNode *node) {}
void CodeGenerator::visit(ConstDeclarationNode *node) {}
void CodeGenerator::visit(VarDeclarationNode *node) {}
void CodeGenerator::visit(TypeDeclarationNode *node) {}

void CodeGenerator::visit(ProcedureDeclarationNode *node)
{
    if (!node) return;

    int skipJmp = getNextAddress();
    emit(OpCode::JMP, 0, 0);

    int procStart = getNextAddress();
    subroutineAddresses[node->tabIndex] = procStart;

    int blockRef = symbolTable->tab[node->tabIndex].ref;
    int totalMem = 3 + symbolTable->btab[blockRef].psze + symbolTable->btab[blockRef].vsze;
    emit(OpCode::INT, 0, totalMem);

    currentLevel++;
    if (node->body) node->body->accept(this);
    currentLevel--;

    emit(OpCode::RET, 0, 0);

    backpatch(skipJmp, getNextAddress());
}

void CodeGenerator::visit(FunctionDeclarationNode *node)
{
    if (!node) return;

    int skipJmp = getNextAddress();
    emit(OpCode::JMP, 0, 0);

    int funcStart = getNextAddress();
    subroutineAddresses[node->tabIndex] = funcStart;

    int blockRef = symbolTable->tab[node->tabIndex].ref;
    int returnValueSize = 1;
    int totalMem = 3 + returnValueSize + symbolTable->btab[blockRef].psze + symbolTable->btab[blockRef].vsze;
    emit(OpCode::INT, 0, totalMem);

    currentLevel++;
    if (node->body) node->body->accept(this);
    currentLevel--;
    
    emit(OpCode::RET, 0, 0);
    backpatch(skipJmp, getNextAddress());
}

void CodeGenerator::visit(ProgramNode *node)
{
    if (!node) return;

    int totalMemory = 3 + symbolTable->btab[0].vsze;
    emit(OpCode::INT, 0, totalMemory);

    for (auto &decl : node->declarations) {
        if (decl) decl->accept(this);
    }

    if (node->statements) {
        node->statements->accept(this);
    }

    emit(OpCode::RET, 0, 0);
}

void CodeGenerator::visit(BlockNode *node)
{
    if (!node) return;
    for (auto &decl : node->declarations) {
        if (decl) decl->accept(this);
    }
    if (node->statements) {
        node->statements->accept(this);
    }
}

void CodeGenerator::visit(CompoundStatementNode *node)
{
    if (!node) return;
    for (auto &stmt : node->statements) {
        if (stmt) stmt->accept(this);
    }
}

void CodeGenerator::visit(EmptyStatementNode *node) {}
void CodeGenerator::visit(IntegerLiteralNode *node) { emitLiteral(0, node->value); }
void CodeGenerator::visit(RealLiteralNode *node) { emitLiteral(0, node->value); }
void CodeGenerator::visit(StringLiteralNode *node) { emitLiteral(0, node->value); }
void CodeGenerator::visit(BooleanLiteralNode *node) { emitLiteral(0, node->value); }
void CodeGenerator::visit(CharLiteralNode *node) { emitLiteral(0, node->value); }

void CodeGenerator::visit(VarAccessNode *node)
{
    if (!node) return;
    emitValue(node);
}

void CodeGenerator::visit(ArrayAccessNode *node)
{
    if (!node) return;
    emitValue(node);
}

void CodeGenerator::visit(FieldAccessNode *node)
{
    if (!node) return;
    emitValue(node);
}

void CodeGenerator::visit(AssignmentStatementNode *node)
{
    if (!node) return;

    emitAddress(node->target.get());
    emitValue(node->value.get());
    emit(OpCode::STA, 0, 0);
}

void CodeGenerator::visit(BinaryOpNode *node)
{
    if (!node) return;
    if (node->left) {
        emitValue(node->left.get());
    }
    if (node->right) {
        emitValue(node->right.get());
    };

    std::string op = node->op;
    std::transform(op.begin(), op.end(), op.begin(), ::tolower);

    int opCode = 0;
    if (op == "+") opCode = 2;
    else if (op == "-") opCode = 3;
    else if (op == "*") opCode = 4;
    else if (op == "/" || op == "div") opCode = 5;
    else if (op == "mod") opCode = 6;
    else if (op == "=" || op == "==") opCode = 7;
    else if (op == "<>") opCode = 8;
    else if (op == "<") opCode = 9;
    else if (op == ">=") opCode = 10;
    else if (op == ">") opCode = 11;
    else if (op == "<=") opCode = 12;
    else if (op == "and") opCode = 4;
    else if (op == "or") opCode = 2;

    if (opCode != 0) emit(OpCode::OPR, 0, opCode);
}

void CodeGenerator::visit(UnaryOpNode *node)
{
    if (!node) return;
    if (node->operand) emitValue(node->operand.get());

    std::string op = node->op;
    std::transform(op.begin(), op.end(), op.begin(), ::tolower);

    if (op == "-") emit(OpCode::OPR, 0, 1);
    else if (op == "not") {
        emitLiteral(0, 0);
        emit(OpCode::OPR, 0, 7); // value == 0
    }
}

void CodeGenerator::visit(IfStatementNode *node)
{
    if (!node) return;
    if (node->condition) emitValue(node->condition.get());

    int jpcIndex = getNextAddress();
    emit(OpCode::JPC, 0, 0);

    if (node->thenBranch) node->thenBranch->accept(this);

    if (node->elseBranch) {
        int jmpIndex = getNextAddress();
        emit(OpCode::JMP, 0, 0);

        backpatch(jpcIndex, getNextAddress());
        node->elseBranch->accept(this);
        backpatch(jmpIndex, getNextAddress());
    }
    else {
        backpatch(jpcIndex, getNextAddress());
    }
}

void CodeGenerator::visit(WhileLoopNode *node)
{
    if (!node) return;
    int loopStartIndex = getNextAddress();

    if (node->condition) emitValue(node->condition.get());

    int jpcIndex = getNextAddress();
    emit(OpCode::JPC, 0, 0);

    if (node->body) node->body->accept(this);

    emit(OpCode::JMP, 0, loopStartIndex);
    backpatch(jpcIndex, getNextAddress());
}

void CodeGenerator::visit(RepeatUntilNode *node)
{
    if (!node) return;

    int loopStartIndex = getNextAddress();

    for (auto &stmt : node->body) {
        if (stmt) stmt->accept(this);
    }

    if (node->condition) emitValue(node->condition.get());

    emit(OpCode::JPC, 0, loopStartIndex);
}

void CodeGenerator::visit(ForLoopNode *node)
{
    if (!node) return;

    int index = node->counterTabIndex;
    if (index == 0) {
        std::cerr << "CodeGen Error: unresolved for-loop counter '" << node->counterVar << "'.\n";
        return;
    }

    int counterAdr = symbolTable->tab[index].adr;
    int diffLevel = currentLevel - symbolTable->tab[index].lev;

    if (node->startValue) emitValue(node->startValue.get());
    emit(OpCode::STO, diffLevel, counterAdr);

    int loopStart = getNextAddress();

    emit(OpCode::LOD, diffLevel, counterAdr);
    if (node->endValue) emitValue(node->endValue.get());
    emit(OpCode::OPR, 0, node->isDownTo ? 10 : 12);

    int exitJmp = getNextAddress();
    emit(OpCode::JPC, 0, 0);

    if (node->body) node->body->accept(this);

    emit(OpCode::LOD, diffLevel, counterAdr);
    emitLiteral(0, 1);
    emit(OpCode::OPR, 0, node->isDownTo ? 3 : 2);
    emit(OpCode::STO, diffLevel, counterAdr);
    emit(OpCode::JMP, 0, loopStart);

    backpatch(exitJmp, getNextAddress());
}

void CodeGenerator::visit(CaseStatementNode *node)
{
    if (!node) return;

    std::vector<int> jmpToEndList;

    for (auto &caseItem : node->cases) {
        int nextCaseJmp = 0;
        for (auto &caseConst : caseItem.first) {
            if (node->expression) emitValue(node->expression.get());
            if (caseConst) emitValue(caseConst.get());

            emit(OpCode::OPR, 0, 7);
            nextCaseJmp = getNextAddress();
            emit(OpCode::JPC, 0, 0);

            if (caseItem.second) caseItem.second->accept(this);

            jmpToEndList.push_back(getNextAddress());
            emit(OpCode::JMP, 0, 0);

            backpatch(nextCaseJmp, getNextAddress());
        }
    }

    if (node->elseBranch) node->elseBranch->accept(this);

    int endOfCase = getNextAddress();
    for (int jmpIdx : jmpToEndList) {
        backpatch(jmpIdx, endOfCase);
    }
}

void CodeGenerator::visit(ProcedureCallNode *node)
{
    if (!node) return;

    std::string funcName = node->name;
    std::transform(funcName.begin(), funcName.end(), funcName.begin(), ::tolower);

    if (funcName == "writeln" || funcName == "write") {
        int oprCode = (funcName == "writeln") ? 14 : 13;
        for (auto &arg : node->args) {
            emitValue(arg.get());
            emit(OpCode::OPR, 0, oprCode);
        }
    }
    else if (funcName == "readln" || funcName == "read") {
        for (auto &arg : node->args) {
            emit(OpCode::OPR, 0, 15);

            if (auto varNode = std::dynamic_pointer_cast<VarAccessNode>(arg)) {
                int index = varNode->tabIndex;
                int lev = symbolTable->tab[index].lev;
                int adr = symbolTable->tab[index].adr;
                
                emit(OpCode::STO, currentLevel - lev, adr);
            } else {
                std::cerr << "CodeGen Error: readln argument must be a variable.\n";
            }
        }
    }
    else {
        for (auto &arg : node->args)
            emitValue(arg.get());

        int callIdx = getNextAddress();
        emit(OpCode::CAL, currentLevel - symbolTable->tab[node->tabIndex].lev, 0);
        instructions[callIdx].argCount = static_cast<int>(node->args.size());
        unresolvedCalls.push_back({callIdx, node->tabIndex});
    }
}

void CodeGenerator::visit(FunctionCallNode *node)
{
    if (!node) return;
    for (auto &arg : node->args)
        emitValue(arg.get());

    int callIdx = getNextAddress();
    emit(OpCode::CAL, currentLevel - symbolTable->tab[node->tabIndex].lev, 0);
    instructions[callIdx].argCount = static_cast<int>(node->args.size());
    instructions[callIdx].returnsValue = true;
    
    unresolvedCalls.push_back({callIdx, node->tabIndex});
}

bool CodeGenerator::exportToFile(const std::string &filename) const
{
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Tidak dapat membuka file " << filename << " untuk ditulis.\n";
        return false;
    }

    for (size_t i = 0; i < instructions.size(); ++i) {
        const auto &instr = instructions[i];

        outFile << i << " ";
        outFile << opCodeToString(instr.op) << " ";
        outFile << instr.level << " ";

        if (instr.op == OpCode::LIT && instr.hasLiteral) {
            if (std::holds_alternative<std::string>(instr.literal)) {
                outFile << "\"" << std::get<std::string>(instr.literal) << "\"";
            }
            else if (std::holds_alternative<char>(instr.literal)) {
                outFile << "'" << std::get<char>(instr.literal) << "'";
            }
            else {
                outFile << runtimeValueToString(instr.literal);
            }
        }
        else {
            outFile << instr.operand;
        }
        outFile << "\n";
    }

    outFile.close();
    return true;
}
