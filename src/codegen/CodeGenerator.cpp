#include "CodeGenerator.hpp"
#include "../semantic/ASTNode.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iomanip>

CodeGenerator::CodeGenerator(const SymbolTable* symTab) : symbolTable(symTab) {}

void CodeGenerator::generate(ProgramNode* root)
{
    instructions.clear();
    subroutineAddresses.clear();
    unresolvedCalls.clear();

    if (root) {
        root->accept(this);
    }
    
    resolveFunctionCalls();
}

const std::vector<Instruction>& CodeGenerator::getInstructions() const {
    return instructions;
}

void CodeGenerator::emit(OpCode op, int level, int operand) {
    instructions.push_back(Instruction(op, level, operand));
}

void CodeGenerator::emitLiteral(int level, RuntimeValue value) {
    instructions.push_back(Instruction(OpCode::LIT, level, value));
}

int CodeGenerator::getNextAddress() const {
    return instructions.size();
}

void CodeGenerator::backpatch(int instructionIndex, int targetAddress) {
    if (instructionIndex >= 0 && instructionIndex < (int)instructions.size()) {
        instructions[instructionIndex].operand = targetAddress;
    }
}

void CodeGenerator::resolveFunctionCalls() {
    for (auto& call : unresolvedCalls) {
        int instrIndex = call.first;
        int targetTabIndex = call.second;
        
        if (subroutineAddresses.find(targetTabIndex) != subroutineAddresses.end()) {
            instructions[instrIndex].operand = subroutineAddresses[targetTabIndex];
        } else {
            std::cerr << "Warning: Unresolved subroutine call to tabIndex " << targetTabIndex << "\n";
        }
    }
}

void CodeGenerator::visit(SimpleTypeNode* node) {}
void CodeGenerator::visit(ArrayTypeNode* node) {}
void CodeGenerator::visit(RecordTypeNode* node) {}
void CodeGenerator::visit(EnumeratedTypeNode* node) {}
void CodeGenerator::visit(RangeTypeNode* node) {}
void CodeGenerator::visit(ConstDeclarationNode* node) {}
void CodeGenerator::visit(VarDeclarationNode* node) {}
void CodeGenerator::visit(TypeDeclarationNode* node) {}

void CodeGenerator::visit(ProcedureDeclarationNode* node) {
    if (!node) return;

    int skipJmp = getNextAddress();
    emit(OpCode::JMP, 0, 0);

    int procStart = getNextAddress();
    subroutineAddresses[node->tabIndex] = procStart;

    int blockRef = symbolTable->tab[node->tabIndex].ref;
    int totalMem = 3 + symbolTable->btab[blockRef].psze + symbolTable->btab[blockRef].vsze;
    emit(OpCode::INT, 0, totalMem);

    if (node->body) node->body->accept(this);

    emit(OpCode::RET, 0, 0);

    backpatch(skipJmp, getNextAddress());
}

void CodeGenerator::visit(FunctionDeclarationNode* node) {
    if (!node) return;

    int skipJmp = getNextAddress();
    emit(OpCode::JMP, 0, 0);

    int funcStart = getNextAddress();
    subroutineAddresses[node->tabIndex] = funcStart;

    int blockRef = symbolTable->tab[node->tabIndex].ref;
    int totalMem = 3 + symbolTable->btab[blockRef].psze + symbolTable->btab[blockRef].vsze;
    emit(OpCode::INT, 0, totalMem);

    if (node->body) node->body->accept(this);

    emit(OpCode::RET, 0, 0);
    backpatch(skipJmp, getNextAddress());
}

void CodeGenerator::visit(ProgramNode* node) {
    if (!node) return;

    int totalMemory = 3 + symbolTable->btab[0].vsze;
    emit(OpCode::INT, 0, totalMemory);

    for (auto& decl : node->declarations) {
        if (decl) decl->accept(this);
    }

    if (node->statements) {
        node->statements->accept(this);
    }

    emit(OpCode::RET, 0, 0);
}

void CodeGenerator::visit(BlockNode* node) {
    if (!node) return;
    for (auto& decl : node->declarations) {
        if (decl) decl->accept(this);
    }
    if (node->statements) {
        node->statements->accept(this);
    }
}

void CodeGenerator::visit(CompoundStatementNode* node) {
    if (!node) return;
    for (auto& stmt : node->statements) {
        if (stmt) stmt->accept(this);
    }
}

void CodeGenerator::visit(EmptyStatementNode* node) {}
void CodeGenerator::visit(IntegerLiteralNode* node) { emitLiteral(0, node->value); }
void CodeGenerator::visit(RealLiteralNode* node) { emitLiteral(0, node->value); }
void CodeGenerator::visit(StringLiteralNode* node) { emitLiteral(0, node->value); }
void CodeGenerator::visit(BooleanLiteralNode* node) { emitLiteral(0, node->value); }
void CodeGenerator::visit(CharLiteralNode* node) { emitLiteral(0, node->value); }

void CodeGenerator::visit(VarAccessNode* node) {
    if (!node) return;
    int index = node->tabIndex;
    if (symbolTable->tab[index].obj == ObjectType::CONSTANT) {
        emitLiteral(0, symbolTable->tab[index].adr);
    } else {
        emit(OpCode::LOD, symbolTable->tab[index].lev, symbolTable->tab[index].adr);
    }
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    if (!node) return;

    if (auto varTarget = std::dynamic_pointer_cast<VarAccessNode>(node->target)) {
        emit(OpCode::LDA, 0, symbolTable->tab[varTarget->tabIndex].adr);
        
        int atabIndex = symbolTable->tab[varTarget->tabIndex].ref;
        int low = symbolTable->atab[atabIndex].low;
        int elsz = symbolTable->atab[atabIndex].elsz;
        node->index->accept(this);
        emitLiteral(0, low);
        emit(OpCode::OPR, 0, 3); // SUB: (Index - Low)
        emitLiteral(0, elsz);
        emit(OpCode::OPR, 0, 4); // MUL: * ElSz
        emit(OpCode::OPR, 0, 2); // ADD: Base + Offset
        emit(OpCode::LDI, 0, 0); 
    }
}

void CodeGenerator::visit(FieldAccessNode* node) {
    if (auto varTarget = std::dynamic_pointer_cast<VarAccessNode>(node->target)) {
        emit(OpCode::LDA, 0, symbolTable->tab[varTarget->tabIndex].adr);
        emitLiteral(0, 0);       // Offset dummy
        emit(OpCode::OPR, 0, 2); // ADD
        emit(OpCode::LDI, 0, 0);
    }
}

void CodeGenerator::visit(AssignmentStatementNode* node) {
    if (!node) return;
    if (auto arrTarget = std::dynamic_pointer_cast<ArrayAccessNode>(node->target)) {
        if (auto varTarget = std::dynamic_pointer_cast<VarAccessNode>(arrTarget->target)) {
            emit(OpCode::LDA, 0, symbolTable->tab[varTarget->tabIndex].adr);
            
            int atabIndex = symbolTable->tab[varTarget->tabIndex].ref;
            int low = symbolTable->atab[atabIndex].low;
            int elsz = symbolTable->atab[atabIndex].elsz;

            arrTarget->index->accept(this);
            emitLiteral(0, low);
            emit(OpCode::OPR, 0, 3);
            emitLiteral(0, elsz);
            emit(OpCode::OPR, 0, 4);
            emit(OpCode::OPR, 0, 2);
            if (node->value) node->value->accept(this);

            emit(OpCode::STI, 0, 0);
        }
    } 
    else if (auto varTarget = std::dynamic_pointer_cast<VarAccessNode>(node->target)) {
        if (node->value) node->value->accept(this);
        emit(OpCode::STO, symbolTable->tab[varTarget->tabIndex].lev, symbolTable->tab[varTarget->tabIndex].adr);
    }
}

void CodeGenerator::visit(BinaryOpNode* node) {
    if (!node) return;
    if (node->left) node->left->accept(this);
    if (node->right) node->right->accept(this);

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

void CodeGenerator::visit(UnaryOpNode* node) {
    if (!node) return;
    if (node->operand) node->operand->accept(this);

    std::string op = node->op;
    std::transform(op.begin(), op.end(), op.begin(), ::tolower);

    if (op == "-") emit(OpCode::OPR, 0, 1);
}

void CodeGenerator::visit(IfStatementNode* node) {
    if (!node) return;
    if (node->condition) node->condition->accept(this);

    int jpcIndex = getNextAddress();
    emit(OpCode::JPC, 0, 0);

    if (node->thenBranch) node->thenBranch->accept(this);

    if (node->elseBranch) {
        int jmpIndex = getNextAddress();
        emit(OpCode::JMP, 0, 0);
        
        backpatch(jpcIndex, getNextAddress());
        node->elseBranch->accept(this);
        backpatch(jmpIndex, getNextAddress());
    } else {
        backpatch(jpcIndex, getNextAddress());
    }
}

void CodeGenerator::visit(WhileLoopNode* node) {
    if (!node) return;
    int loopStartIndex = getNextAddress();

    if (node->condition) node->condition->accept(this);
    
    int jpcIndex = getNextAddress();
    emit(OpCode::JPC, 0, 0);

    if (node->body) node->body->accept(this);

    emit(OpCode::JMP, 0, loopStartIndex);
    backpatch(jpcIndex, getNextAddress());
}

void CodeGenerator::visit(RepeatUntilNode* node) {
    if (!node) return;

    int loopStartIndex = getNextAddress();

    for (auto& stmt : node->body) {
        if (stmt) stmt->accept(this);
    }

    if (node->condition) node->condition->accept(this);

    emit(OpCode::JPC, 0, loopStartIndex);
}

void CodeGenerator::visit(ForLoopNode* node) {
    if (!node) return;

    int index = symbolTable->lookup(node->counterVar, symbolTable->btab[node->scopeLevel].last);
    int counterAdr = symbolTable->tab[index].adr;
    int counterLev = symbolTable->tab[index].lev;

    if (node->startValue) node->startValue->accept(this);
    emit(OpCode::STO, counterLev, counterAdr);

    int loopStart = getNextAddress();

    emit(OpCode::LOD, counterLev, counterAdr);
    if (node->endValue) node->endValue->accept(this);
    emit(OpCode::OPR, 0, node->isDownTo ? 10 : 12);

    int exitJmp = getNextAddress();
    emit(OpCode::JPC, 0, 0);

    if (node->body) node->body->accept(this);

    emit(OpCode::LOD, counterLev, counterAdr);
    emitLiteral(0, 1);
    emit(OpCode::OPR, 0, node->isDownTo ? 3 : 2);
    emit(OpCode::STO, counterLev, counterAdr);
    emit(OpCode::JMP, 0, loopStart);
    
    backpatch(exitJmp, getNextAddress());
}

void CodeGenerator::visit(CaseStatementNode* node) {
    if (!node) return;

    std::vector<int> jmpToEndList;

    for (auto& caseItem : node->cases) {
        int nextCaseJmp = 0;
        for (auto& caseConst : caseItem.first) {
            if (node->expression) node->expression->accept(this);
            if (caseConst) caseConst->accept(this);
            
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


void CodeGenerator::visit(ProcedureCallNode* node) {
    if (!node) return;

    std::string funcName = node->name;
    std::transform(funcName.begin(), funcName.end(), funcName.begin(), ::tolower);

    if (funcName == "writeln" || funcName == "write") {
        int oprCode = (funcName == "writeln") ? 14 : 13;
        for (auto& arg : node->args) {
            arg->accept(this);
            emit(OpCode::OPR, 0, oprCode);
        }
    } 
    else if (funcName == "readln" || funcName == "read") {
        for (auto& arg : node->args) {
            emit(OpCode::OPR, 0, 15); 
            
            if (auto varNode = std::dynamic_pointer_cast<VarAccessNode>(arg)) {
                int index = varNode->tabIndex;
                int lev = symbolTable->tab[index].lev;
                int adr = symbolTable->tab[index].adr;
                
                emit(OpCode::STO, lev, adr);
            } else {
                std::cerr << "CodeGen Error: readln argument must be a variable.\n";
            }
        }
    }
    else {
        for (auto& arg : node->args) arg->accept(this);
        
        int callIdx = getNextAddress();
        emit(OpCode::CAL, symbolTable->tab[node->tabIndex].lev, 0);
        unresolvedCalls.push_back({callIdx, node->tabIndex});
    }
}

void CodeGenerator::visit(FunctionCallNode* node) {
    if (!node) return;
    for (auto& arg : node->args) arg->accept(this);
    
    int callIdx = getNextAddress();
    emit(OpCode::CAL, symbolTable->tab[node->tabIndex].lev, 0);
    unresolvedCalls.push_back({callIdx, node->tabIndex});
}

bool CodeGenerator::exportToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Tidak dapat membuka file " << filename << " untuk ditulis.\n";
        return false;
    }

    for (size_t i = 0; i < instructions.size(); ++i) {
        const auto& instr = instructions[i];
        
        outFile << i << " ";
        outFile << opCodeToString(instr.op) << " ";
        outFile << instr.level << " ";
        
        if (instr.op == OpCode::LIT && instr.hasLiteral) {
            if (std::holds_alternative<std::string>(instr.literal)) {
                outFile << "\"" << std::get<std::string>(instr.literal) << "\"";
            } else if (std::holds_alternative<char>(instr.literal)) {
                outFile << "'" << std::get<char>(instr.literal) << "'";
            } else {
                outFile << runtimeValueToString(instr.literal);
            }
        } else {
            outFile << instr.operand;
        }
        outFile << "\n";
    }

    outFile.close();
    return true;
}