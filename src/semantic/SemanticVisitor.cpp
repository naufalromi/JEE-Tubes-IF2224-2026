#include "SemanticVisitor.hpp"
#include <algorithm>
#include <string>

void SemanticVisitor::reportError(ASTNode *node, const std::string &message)
{
    SemanticError err = SemanticError(message, node->line, node->column);
    errors.push_back(err);
}

/**
 * Visit Head Program
 */
void SemanticVisitor::visit(ProgramNode *node)
{
    currentLevel = 0;
    symbolTable.currentBlock = 0;
    int globalBlockStart = symbolTable.btab[0].last;

    // Proses semua deklarasi (termasuk variabel, tipe, prosedur, dan fungsi)
    for (auto &decl : node->declarations) {
        decl->accept(this);
    }

    symbolTable.currentBlock = 0;

    // Hitung ukuran memori variabel global (vsze)
    int totalGlobalMemory = 0;
    int curr = symbolTable.btab[0].last;

    while (curr > globalBlockStart) {
        if (symbolTable.tab[curr].lev == 0 && symbolTable.tab[curr].obj == ObjectType::VARIABLE) {
            if (symbolTable.tab[curr].type == DataType::ARRAY) {
                totalGlobalMemory += symbolTable.atab[symbolTable.tab[curr].ref].size;
            } else if (symbolTable.tab[curr].type == DataType::RECORD) {
                totalGlobalMemory += symbolTable.btab[symbolTable.tab[curr].ref].vsze;
            } else {
                totalGlobalMemory += getDataTypeSize(symbolTable.tab[curr].type);
            }   
        }
        curr = symbolTable.tab[curr].link;
    }
    
    // Simpan ukuran total memori variabel global
    symbolTable.btab[0].vsze = totalGlobalMemory;
    symbolTable.btab[0].psze = 0;
    symbolTable.btab[0].lpar = 0; 

    if (node->statements) {
        node->statements->accept(this);
    }
}

void SemanticVisitor::visit(VarDeclarationNode *node)
{
    if (!node) return;

    if (!node->typeDefinition) {
        reportError(node, "Variable declaration has no type");
        return;
    }

    node->typeDefinition->accept(this);
    DataType type = node->typeDefinition->resolvedType;

    int index = symbolTable.enter(node->name, ObjectType::VARIABLE, type, currentLevel);

    if (index != -1) {
        node->scopeLevel = currentLevel;
        symbolTable.tab[index].ref = node->typeDefinition->resolvedRef; 
    } else {
        reportError(node, "Duplicate identifier: " + node->name);
    }
}

/**
 * block.node = new BlockNode(declaration_part.node_list, compound_statement.node)
 *
 * BlockNode berisi:
 * - declarations: list of declaration nodes (Const, Type, Var, Procedure, Function)
 * - statements: compound statement (begin...end)
 *
 * Proses:
 * 1. Create block table entry for this block (main compound block)
 * 2. Process all declarations in this block
 * 3. Process compound statement body
 */
void SemanticVisitor::visit(BlockNode *node)
{
    if (!node) return;

    node->scopeLevel = currentLevel;

    // Process all declarations in this block (const, type, var, nested proc/func)
    for (auto &decl : node->declarations) {
        decl->accept(this);
    }

    // Process compound statement body
    if (node->statements) {
        node->statements->accept(this);
    }
}

// ============= DECLARATIONS =============

void SemanticVisitor::visit(ConstDeclarationNode *node)
{
    if (!node) return;

    // Evaluate constant value
    node->value->accept(this);

    // Enter constant into symbol table
    DataType type = node->value->evaluatedType;
    int index = symbolTable.enter(node->name, ObjectType::CONSTANT, type, currentLevel);

    if (index == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
    }
    else {
        node->scopeLevel = currentLevel;
    }
}

void SemanticVisitor::visit(TypeDeclarationNode *node)
{
    if (!node) return;

    if (!node->typeDefinition) {
        reportError(node, "Type declaration has no definition");
        return;
    }

    // Process type definition to resolve its type
    node->typeDefinition->accept(this);

    // Enter type into symbol table with its resolved type
    DataType resolvedType = node->typeDefinition->resolvedType;
    int index = symbolTable.enter(node->name, ObjectType::TYPE, resolvedType, currentLevel);

    if (index == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
    }
    else {
        node->scopeLevel = currentLevel;
        symbolTable.tab[index].ref = node->typeDefinition->resolvedRef;
    }
}

void SemanticVisitor::visit(ProcedureDeclarationNode *node)
{
if (!node) return;

    // Tambahkan prosedur ke scope parent
    int procIndex = symbolTable.enter(node->name, ObjectType::PROCEDURE, DataType::UNKNOWN, currentLevel);
    if (procIndex == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
        return;
    }
    node->scopeLevel = currentLevel;

    // Siapkan Block baru untuk Prosedur
    BtabEntry procBlock;
    procBlock.last = procIndex;
    procBlock.lpar = -1;
    procBlock.psze = 0;
    procBlock.vsze = 0;
    symbolTable.btab.push_back(procBlock);

    // Pindah masuk ke Scope Prosedur
    currentLevel++;
    int prevBlock = symbolTable.currentBlock; // Simpan alamat scope parent
    symbolTable.currentBlock = symbolTable.btab.size() - 1; // Pindah ke block baru

    int initialLastIndex = procBlock.last;

    // Visit Parameter
    for (auto &param : node->parameters) { 
        param->accept(this); 
    }

    // Hitung Parameter Metrics (lpar & psze)
    int paramLastIndex = symbolTable.btab[symbolTable.currentBlock].last;
    int totalParamMemory = 0;
    int currParam = paramLastIndex;

    while (currParam > initialLastIndex) {
        if (symbolTable.tab[currParam].obj == ObjectType::VARIABLE) {
            if (symbolTable.tab[currParam].type == DataType::ARRAY) {
                int arrayRef = symbolTable.tab[currParam].ref;
                totalParamMemory += symbolTable.atab[arrayRef].size;
            } else if (symbolTable.tab[currParam].type == DataType::RECORD) {
                int recordRef = symbolTable.tab[currParam].ref;
                totalParamMemory += symbolTable.btab[recordRef].vsze;
            } else {
                totalParamMemory += getDataTypeSize(symbolTable.tab[currParam].type);
            }
        }
        currParam = symbolTable.tab[currParam].link;
    }
    symbolTable.btab[symbolTable.currentBlock].lpar = paramLastIndex;
    symbolTable.btab[symbolTable.currentBlock].psze = totalParamMemory;

    // Visit Body (Variables & Statements)
    if (node->body) { 
        node->body->accept(this); 
    }

    // Hitung Local Variables Metrics (vsze)
    int finalLastIndex = symbolTable.btab[symbolTable.currentBlock].last;
    int totalLocalMemory = 0;
    int currLocal = finalLastIndex;

    while (currLocal > paramLastIndex) {
        if (symbolTable.tab[currLocal].obj == ObjectType::VARIABLE) {
            if (symbolTable.tab[currLocal].type == DataType::ARRAY) {
                int arrayRef = symbolTable.tab[currLocal].ref;
                totalLocalMemory += symbolTable.atab[arrayRef].size;
            } else if (symbolTable.tab[currLocal].type == DataType::RECORD) {
                int recordRef = symbolTable.tab[currLocal].ref;
                totalLocalMemory += symbolTable.btab[recordRef].vsze;
            } else {
                totalLocalMemory += getDataTypeSize(symbolTable.tab[currLocal].type);
            }
        }
        currLocal = symbolTable.tab[currLocal].link;
    }
    node->localVariablesSize = totalLocalMemory;
    symbolTable.btab[symbolTable.currentBlock].vsze = totalLocalMemory;

    // Keluar dari Scope Prosedur
    symbolTable.currentBlock = prevBlock;
    currentLevel--;
}

void SemanticVisitor::visit(FunctionDeclarationNode *node)
{
if (!node) return;

    node->returnType->accept(this);
    DataType returnType = node->returnType->resolvedType;
    
    // Tambahkan fungsi ke scope parent
    int funcIndex = symbolTable.enter(node->name, ObjectType::FUNCTION, returnType, currentLevel);
    if (funcIndex == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
        return;
    }
    node->scopeLevel = currentLevel;

    // Siapkan block baru untuk Fungsi
    BtabEntry funcBlock;
    funcBlock.last = funcIndex;
    funcBlock.lpar = -1;
    funcBlock.psze = 0;
    funcBlock.vsze = 0;
    symbolTable.btab.push_back(funcBlock);

    // Pindah masuk ke Scope Fungsi
    currentLevel++;
    int prevBlock = symbolTable.currentBlock;
    symbolTable.currentBlock = symbolTable.btab.size() - 1;

    // Register function name sebagai local variable
    int nameIndex = symbolTable.enter(node->name, ObjectType::VARIABLE, returnType, currentLevel);
    if (nameIndex == -1) {
        reportError(node, "Failed to register function name in local scope: " + node->name);
    }

    int initialLastIndex = symbolTable.btab[symbolTable.currentBlock].last;

    // Visit Parameter
    for (auto &param : node->parameters) { 
        param->accept(this); 
    }

    // Hitung Parameter Metrics
    int paramLastIndex = symbolTable.btab[symbolTable.currentBlock].last;
    int totalParamMemory = 0;
    int currParam = paramLastIndex;

    while (currParam > initialLastIndex) {
        if (symbolTable.tab[currParam].obj == ObjectType::VARIABLE) {
            if (symbolTable.tab[currParam].type == DataType::ARRAY) {
                int arrayRef = symbolTable.tab[currParam].ref;
                totalParamMemory += symbolTable.atab[arrayRef].size;
            } else if (symbolTable.tab[currParam].type == DataType::RECORD) {
                int recordRef = symbolTable.tab[currParam].ref;
                totalParamMemory += symbolTable.btab[recordRef].vsze;
            } else {
                totalParamMemory += getDataTypeSize(symbolTable.tab[currParam].type);
            }
        }
        currParam = symbolTable.tab[currParam].link;
    }
    symbolTable.btab[symbolTable.currentBlock].lpar = paramLastIndex;
    symbolTable.btab[symbolTable.currentBlock].psze = totalParamMemory;

    // Visit Body
    if (node->body) { 
        node->body->accept(this); 
    }

    // Hitung Local Variables Metrics
    int finalLastIndex = symbolTable.btab[symbolTable.currentBlock].last;
    int totalLocalMemory = 0;
    int currLocal = finalLastIndex;

    while (currLocal > paramLastIndex) {
        if (symbolTable.tab[currLocal].obj == ObjectType::VARIABLE) {
            if (symbolTable.tab[currLocal].type == DataType::ARRAY) {
                int arrayRef = symbolTable.tab[currLocal].ref;
                totalLocalMemory += symbolTable.atab[arrayRef].size;
            } else if (symbolTable.tab[currLocal].type == DataType::RECORD) {
                int recordRef = symbolTable.tab[currLocal].ref;
                totalLocalMemory += symbolTable.btab[recordRef].vsze;
            } else {
                totalLocalMemory += getDataTypeSize(symbolTable.tab[currLocal].type);
            }
        }
        currLocal = symbolTable.tab[currLocal].link;
    }
    node->localVariablesSize = totalLocalMemory;
    symbolTable.btab[symbolTable.currentBlock].vsze = totalLocalMemory;

    // Exit scope
    symbolTable.currentBlock = prevBlock;
    currentLevel--;
}

// ============= TYPES =============

void SemanticVisitor::visit(SimpleTypeNode *node)
{
    if (!node) return;

    // Convert to lowercase for case-insensitive comparison
    std::string typeName = node->name;
    std::transform(typeName.begin(), typeName.end(), typeName.begin(), ::tolower);

    // Resolve simple type name to DataType
    if (typeName == "integer") {
        node->resolvedType = DataType::INTEGER;
    }
    else if (typeName == "real") {
        node->resolvedType = DataType::REAL;
    }
    else if (typeName == "boolean") {
        node->resolvedType = DataType::BOOLEAN;
    }
    else if (typeName == "char") {
        node->resolvedType = DataType::CHAR;
    }
    else if (typeName == "string") {
        node->resolvedType = DataType::STRING;
    }
    else {
        // User-defined type - lookup in symbol table
        int index = symbolTable.lookup(node->name, symbolTable.btab[symbolTable.currentBlock].last);
        if (index != 0) {
            node->resolvedType = symbolTable.tab[index].type;
            node->resolvedRef = symbolTable.tab[index].ref;
        }
        else {
            reportError(node, "Undefined type: " + node->name);
            node->resolvedType = DataType::UNKNOWN;
        }
    }
}

void SemanticVisitor::visit(ArrayTypeNode *node)
{
    if (!node) return;

    if (node->resolvedType != DataType::UNKNOWN) {
        return; 
    }

    int elementAtabRef = 0;  // Reference to element type in symbol table if composite
    DataType indexType = DataType::UNKNOWN;
    DataType elementType = DataType::UNKNOWN;
    int low = 0, high = 0;
    int elementSize = 0;

    // Process index type (usually a range)
    if (node->indexType) {
        node->indexType->accept(this);
        
        // If index is a subrange, extract bounds from it directly
        if (auto rangeType = std::dynamic_pointer_cast<RangeTypeNode>(node->indexType)) {
            // Determine bound type from lower bound
            if (rangeType->lowBound) {
                indexType = rangeType->lowBound->evaluatedType;  // Get base type (INTEGER, CHAR, etc.)
                
                if (auto intLit = std::dynamic_pointer_cast<IntegerLiteralNode>(rangeType->lowBound)) {
                    low = intLit->value;
                }
                else if (auto charLit = std::dynamic_pointer_cast<CharLiteralNode>(rangeType->lowBound)) {
                    low = (int)charLit->value;
                }
            }
            
            // Extract upper bound
            if (rangeType->highBound) {
                if (auto intLit = std::dynamic_pointer_cast<IntegerLiteralNode>(rangeType->highBound)) {
                    high = intLit->value;
                }
                else if (auto charLit = std::dynamic_pointer_cast<CharLiteralNode>(rangeType->highBound)) {
                    high = (int)charLit->value;
                }
            }
        } else {
            // Simple type index 
            indexType = node->indexType->resolvedType;
        }
    }

    // Process element type
    if (node->elementType) {
        node->elementType->accept(this);
        elementType = node->elementType->resolvedType;
        if (elementType == DataType::ARRAY) {
            elementSize = symbolTable.atab[node->elementType->resolvedRef].size;
        } else if (elementType == DataType::RECORD) {
            elementSize = symbolTable.btab[node->elementType->resolvedRef].vsze;
        } else {
            elementSize = getDataTypeSize(elementType);
        }
        elementAtabRef = node->elementType->resolvedRef; 
    }

    // Create SINGLE atab entry for this array type
    AtabEntry arrayEntry;
    arrayEntry.xtyp = indexType;          // Base index type (INTEGER, CHAR, ENUMERATED, etc., NOT SUBRANGE)
    arrayEntry.etyp = elementType;        // Element type
    arrayEntry.eref = elementAtabRef;     // Reference to element type in symbol table if composite
    arrayEntry.low = low;                 // Lower bound
    arrayEntry.high = high;               // Upper bound
    arrayEntry.elsz = elementSize;        // Element size
    arrayEntry.size = (high - low + 1) * elementSize;  // Total size
    
    // Add to array table
    symbolTable.atab.push_back(arrayEntry);
    
    node->resolvedType = DataType::ARRAY; 
    node->resolvedRef = symbolTable.atab.size() - 1;
    node->atabRef = symbolTable.atab.size() - 1;
}

void SemanticVisitor::visit(RecordTypeNode *node)
{
    if (!node) return;
    if (node->resolvedType != DataType::UNKNOWN) return;

    BtabEntry recBlock;
    recBlock.last = 0;
    recBlock.lpar = -1;
    recBlock.psze = 0;
    recBlock.vsze = 0;
    symbolTable.btab.push_back(recBlock);

    int prevBlock = symbolTable.currentBlock;
    symbolTable.currentBlock = symbolTable.btab.size() - 1;

    int startIdx = symbolTable.tabTop;

    for (auto &field : node->fields) {
        if (field) field->accept(this);
    }
    
    int endIdx = symbolTable.tabTop;

    int totalRecordSize = 0;
    int currField = symbolTable.btab[symbolTable.currentBlock].last;
    while (currField > 0 && currField > startIdx) {
        if (symbolTable.tab[currField].obj == ObjectType::VARIABLE) {
            if (symbolTable.tab[currField].type == DataType::ARRAY) {
                totalRecordSize += symbolTable.atab[symbolTable.tab[currField].ref].size;
            } else if (symbolTable.tab[currField].type == DataType::RECORD) {
                totalRecordSize += symbolTable.btab[symbolTable.tab[currField].ref].vsze;
            } else {
                totalRecordSize += getDataTypeSize(symbolTable.tab[currField].type);
            }
        }
        currField = symbolTable.tab[currField].link;
    }

    symbolTable.btab[symbolTable.currentBlock].vsze = totalRecordSize;
    node->btabRef = symbolTable.currentBlock;
    symbolTable.currentBlock = prevBlock;
    node->resolvedType = DataType::RECORD;
    node->resolvedRef = symbolTable.currentBlock;
}

void SemanticVisitor::visit(EnumeratedTypeNode *node)
{
    if (!node) return;
    int enumValue = 0;
    for (const auto& element : node->elements) {
        int index = symbolTable.enter(element, ObjectType::CONSTANT, DataType::ENUMERATED, currentLevel);
        if (index == -1) {
            reportError(node, "Duplicate identifier in enum: " + element);
        } else {
            symbolTable.tab[index].adr = enumValue++; 
        }
    }
    node->resolvedType = DataType::ENUMERATED;
}

void SemanticVisitor::visit(RangeTypeNode *node)
{
    if (!node) return;

    DataType baseType = DataType::UNKNOWN;

    // Process bounds - just evaluate them, don't create atab entry yet
    if (node->lowBound) {
        node->lowBound->accept(this);
        baseType = node->lowBound->evaluatedType;  // Get base type (INTEGER, CHAR, etc.)
    }

    if (node->highBound) {
        node->highBound->accept(this);
    }

    // Store the base type (used by ArrayTypeNode to create atab entry)
    node->baseType = baseType; 
    node->resolvedType = DataType::SUBRANGE;
}

// ============= EXPRESSIONS =============

void SemanticVisitor::visit(IntegerLiteralNode *node)
{
    if (!node) return;
    node->evaluatedType = DataType::INTEGER;
}

void SemanticVisitor::visit(RealLiteralNode *node)
{
    if (!node) return;
    node->evaluatedType = DataType::REAL;
}

void SemanticVisitor::visit(StringLiteralNode *node)
{
    if (!node) return;
    node->evaluatedType = DataType::STRING;
}

void SemanticVisitor::visit(CharLiteralNode *node)
{
    if (!node) return;
    node->evaluatedType = DataType::CHAR;
}

void SemanticVisitor::visit(BooleanLiteralNode *node)
{
    if (!node) return;
    node->evaluatedType = DataType::BOOLEAN;
}

void SemanticVisitor::visit(VarAccessNode *node)
{
    if (!node) return;

    // Look up variable in symbol table
    int index = symbolTable.lookup(node->name, symbolTable.btab[symbolTable.currentBlock].last);
    if (index == 0) {
        reportError(node, "Undefined identifier: " + node->name);
        node->evaluatedType = DataType::UNKNOWN;
    }
    else {
        node->evaluatedType = symbolTable.tab[index].type;
        node->evaluatedRef = symbolTable.tab[index].ref;
        // Check constant
        if (symbolTable.tab[index].obj == ObjectType::CONSTANT) {
            node->isConstant = true;
        }
    }
}

void SemanticVisitor::visit(ArrayAccessNode *node)
{
if (!node) return;

    if (node->target) {
        node->target->accept(this);
        node->evaluatedType = node->target->evaluatedType; 
        
        int atabIndex = node->target->evaluatedRef;
        if (atabIndex != 0) {
            node->evaluatedRef = symbolTable.atab[atabIndex].eref;
        }
    }

    if (node->index) {
        node->index->accept(this);
        if (node->index->evaluatedType != DataType::INTEGER) {
            reportError(node, "Array index must be integer");
        }
    }
}

void SemanticVisitor::visit(FieldAccessNode *node)
{
    if (!node) return;

    // Evaluasi Target (Bisa berupa variabel 's1' atau array 'siswaBaru[1]')
    if (node->target) {
        node->target->accept(this);
        if (node->target->evaluatedType != DataType::RECORD) {
            reportError(node, "Can only access fields of record type");
            node->evaluatedType = DataType::UNKNOWN;
            return;
        }
    }

    // Ambil referensi BTAB yang sudah dibawa target
    int recordBtabIndex = node->target->evaluatedRef; 
    
    // Cari field di dalam blok spesifik record tersebut
    int index = 0;
    if (recordBtabIndex != 0) {
        index = symbolTable.lookup(node->fieldName, symbolTable.btab[recordBtabIndex].last);
    }
    
    if (index == 0) {
        reportError(node, "Undefined record field: " + node->fieldName);
        node->evaluatedType = DataType::UNKNOWN;
    } else {
        node->evaluatedType = symbolTable.tab[index].type;
        node->evaluatedRef = symbolTable.tab[index].ref;
    }
}

void SemanticVisitor::visit(BinaryOpNode *node)
{
    if (!node) return;

    DataType leftType = DataType::UNKNOWN;
    DataType rightType = DataType::UNKNOWN;

    // Evaluate both operands
    if (node->left) {
        node->left->accept(this);
        leftType = node->left->evaluatedType;
    } else {
        reportError(node, "Internal Error: Left operand is missing in binary operation");
    }
    if (node->right) {
        node->right->accept(this);
        rightType = node->right->evaluatedType;
    } else {
        reportError(node, "Internal Error: Right operand is missing in binary operation");
    }

    // Type checking based on operator
    node->evaluatedType = resolveBinaryType(node->op, leftType, rightType);
}

void SemanticVisitor::visit(UnaryOpNode *node)
{
    if (!node) return;

    // Evaluate operand
    if (node->operand) {
        node->operand->accept(this);
        node->evaluatedType = node->operand->evaluatedType;
    }
}

void SemanticVisitor::visit(FunctionCallNode *node)
{
    if (!node) return;

    int searchLimit = symbolTable.btab[symbolTable.currentBlock].last;
    int index = 0;

    int i = searchLimit;
    while (i > 0) {
        if (symbolTable.tab[i].name == node->name && symbolTable.tab[i].obj == ObjectType::FUNCTION) {
            index = i;
            break;
        }
        i = symbolTable.tab[i].link;
    }

    if (index == 0) {
        reportError(node, "Undefined function: " + node->name);
        node->evaluatedType = DataType::UNKNOWN;
        return;
    }

    // Evaluate all arguments
    for (auto &arg : node->args) {
        if (arg) {
            arg->accept(this);
        }
    }

    // Function return type
    node->evaluatedType = symbolTable.tab[index].type;
}

// ============= STATEMENTS =============

void SemanticVisitor::visit(CompoundStatementNode *node)
{
    if (!node) return;

    // Process all statements in sequence
    for (auto &stmt : node->statements) {
        if (stmt) {
            stmt->accept(this);
        }
    }
}

void SemanticVisitor::visit(AssignmentStatementNode *node)
{
    if (!node) return;

    // Evaluate right-hand side (value)
    if (node->value) {
        node->value->accept(this);
    }

    // Evaluate left-hand side (target)
    if (node->target) {
        node->target->accept(this);

        if (auto varAccess = std::dynamic_pointer_cast<VarAccessNode>(node->target)) {
            // Constant check
            if (varAccess->isConstant) {
                reportError(node, "Illegal assignment: '" + varAccess->name + "' is a constant and cannot be modified");
            }
            // Loop counter check
            if (std::find(activeLoopCounters.begin(), activeLoopCounters.end(), varAccess->name) != activeLoopCounters.end()) {
                reportError(node, "Illegal assignment: FOR loop counter '" + varAccess->name + "' cannot be modified");
            }
        }

        // Type compatibility check
        if (!isCompatible(node->target->evaluatedType, node->value->evaluatedType)) {
            reportError(node, "Type mismatch in assignment");
        }
    }
}

void SemanticVisitor::visit(IfStatementNode *node)
{
    if (!node) return;

    // Evaluate condition (must be boolean)
    if (node->condition) {
        node->condition->accept(this);
        if (node->condition->evaluatedType != DataType::BOOLEAN) {
            reportError(node, "If condition must be boolean");
        }
    }

    // Execute then branch
    if (node->thenBranch) {
        node->thenBranch->accept(this);
    }

    // Execute else branch if present
    if (node->elseBranch) {
        node->elseBranch->accept(this);
    }
}

void SemanticVisitor::visit(CaseStatementNode *node)
{
    if (!node) return;

    // Save data type of main expression
    DataType exprType = DataType::UNKNOWN;
    if (node->expression) {
        node->expression->accept(this);
        exprType = node->expression->evaluatedType;
    }

    // Process each case
    for (auto &caseItem : node->cases) {
        // Check constants
        for (auto &caseConst : caseItem.first) {
            if (caseConst) {
                caseConst->accept(this);
                // Check data type
                DataType constType = caseConst->evaluatedType;
                if (exprType != DataType::UNKNOWN && constType != DataType::UNKNOWN) {
                    if (!isCompatible(exprType, constType)) {
                        reportError(node, "Type mismatch: case label type does not match case expression type");
                    }
                }
            }
        }
        // Execute statement
        if (caseItem.second) {
            caseItem.second->accept(this);
        }
    }

    // Process else branch if present
    if (node->elseBranch) {
        node->elseBranch->accept(this);
    }
}

void SemanticVisitor::visit(WhileLoopNode *node)
{
    if (!node) return;

    // Evaluate condition (must be boolean)
    if (node->condition) {
        node->condition->accept(this);
        if (node->condition->evaluatedType != DataType::BOOLEAN) {
            reportError(node, "While condition must be boolean");
        }
    }

    // Execute loop body
    if (node->body) {
        node->body->accept(this);
    }
}

void SemanticVisitor::visit(ForLoopNode *node)
{
    if (!node) return;

    // Look up counter variable
    int index = symbolTable.lookup(node->counterVar, symbolTable.btab[symbolTable.currentBlock].last);

    if (index == 0) {
        reportError(node, "Undefined loop variable: " + node->counterVar);
    }
    else if (symbolTable.tab[index].type != DataType::INTEGER) {
        reportError(node, "For loop counter must be integer");
    }

    // Evaluate start value (must be integer)
    if (node->startValue) {
        node->startValue->accept(this);
        if (node->startValue->evaluatedType != DataType::INTEGER) {
            reportError(node, "For loop start value must be integer");
        }
    }

    // Evaluate end value (must be integer)
    if (node->endValue) {
        node->endValue->accept(this);
        if (node->endValue->evaluatedType != DataType::INTEGER) {
            reportError(node, "For loop end value must be integer");
        }
    }

    activeLoopCounters.push_back(node->counterVar);

    // Execute loop body
    if (node->body) {
        node->body->accept(this);
    }

    activeLoopCounters.pop_back();
}

void SemanticVisitor::visit(RepeatUntilNode *node)
{
    if (!node) return;

    // Execute loop body
    for (auto &stmt : node->body) {
        if (stmt) {
            stmt->accept(this);
        }
    }

    // Evaluate condition (must be boolean)
    if (node->condition) {
        node->condition->accept(this);
        if (node->condition->evaluatedType != DataType::BOOLEAN) {
            reportError(node, "Repeat-until condition must be boolean");
        }
    }
}

void SemanticVisitor::visit(ProcedureCallNode *node)
{
    if (!node) return;

    int searchLimit = symbolTable.btab[symbolTable.currentBlock].last;
    int index = 0;

    int i = searchLimit;
    while (i > 0) {
        if (symbolTable.tab[i].name == node->name && symbolTable.tab[i].obj == ObjectType::PROCEDURE) {
            index = i;
            break;
        }
        i = symbolTable.tab[i].link;
    }

    if (index == 0) {
        reportError(node, "Undefined procedure: " + node->name);
        return;
    }

    // Evaluate all arguments
    for (auto &arg : node->args) {
        if (arg) {
            arg->accept(this);
        }
    }
}

void SemanticVisitor::visit(EmptyStatementNode *node)
{
    if (!node) return;
    // Empty statement does nothing
}

// ============= HELPER METHODS =============

bool SemanticVisitor::isCompatible(DataType target, DataType source)
{
    // Same type is always compatible
    if (target == source) return true;

    // Allow some implicit conversions
    if (target == DataType::REAL && source == DataType::INTEGER) return true;
    if (target == DataType::STRING && source == DataType::CHAR) return true;

    if (target == DataType::SUBRANGE && source == DataType::INTEGER) return true;
    if (target == DataType::INTEGER && source == DataType::SUBRANGE) return true;

    return false;
}

DataType SemanticVisitor::resolveBinaryType(const std::string &op, DataType left, DataType right)
{
    std::string lowerOp = op;
    std::transform(lowerOp.begin(), lowerOp.end(), lowerOp.begin(), ::tolower);

    // Arithmetic operators: +, -, *, /, mod, div
    if (lowerOp == "+" || lowerOp == "-" || lowerOp == "*" || lowerOp == "/" || lowerOp == "mod" || lowerOp == "div") {
        if ((left == DataType::INTEGER || left == DataType::REAL) &&
            (right == DataType::INTEGER || right == DataType::REAL)) {
            // Result is REAL if either operand is REAL, otherwise INTEGER
            return (left == DataType::REAL || right == DataType::REAL) ? DataType::REAL : DataType::INTEGER;
        }
    }

    // Comparison operators: ==, <>, <, <=, >, >=
    if (lowerOp == "==" || lowerOp == "=" || lowerOp == "<>" || lowerOp == "<" || lowerOp == "<=" || lowerOp == ">" || lowerOp == ">=") {
        return DataType::BOOLEAN;
    }

    // Logical operators: and, or
    if (lowerOp == "and" || lowerOp == "or") {
        if (left == DataType::BOOLEAN && right == DataType::BOOLEAN) {
            return DataType::BOOLEAN;
        }
    }

    return DataType::UNKNOWN;
}

int SemanticVisitor::getDataTypeSize(DataType type)
{
    switch (type) {
        case DataType::INTEGER:
            return 4;
        case DataType::REAL:
            return 8;
        case DataType::BOOLEAN:
            return 1;
        case DataType::CHAR:
            return 1;
        case DataType::STRING:
            return 256; // Assuming max 256 chars
        case DataType::SUBRANGE:
            return 4;  // SUBRANGE typically uses INTEGER as base (4 bytes)
        case DataType::ENUMERATED:
            return 4;  // ENUMERATED typically uses INTEGER (4 bytes)
        case DataType::RECORD:
            return 256;  // Estimated record size
        case DataType::ARRAY:
            return 256;  // Estimated array size
        default:
            return 0;
    }
}

void SemanticVisitor::printErrors() const
{
    if (errors.empty()) {
        std::cout << "No semantic errors found.\n";
        return;
    }

    std::cout << "Semantic Errors:\n";
    for (const auto &err : errors) {
        std::cout << "  Line " << err.getLine() << ", Column " << err.getColumn()
                  << ": " << err.what() << std::endl;
    }
}
