#include "SemanticVisitor.hpp"
#include <algorithm>
#include <string>

void SemanticVisitor::reportError(ASTNode *node, const std::string &message)
{
    SemanticError err = SemanticError(message, node->line, node->column);
    errors.push_back(err);
}

void SemanticVisitor::visit(ProgramNode *node)
{
    currentLevel = 0;
    symbolTable.currentBlock = 0;
    currentVisibilityBlock = 0;
    int globalBlockStart = symbolTable.btab[0].last;

    // Proses semua deklarasi (termasuk variabel, tipe, prosedur, dan fungsi)
    for (auto &decl : node->declarations) {
        decl->accept(this);
    }

    symbolTable.currentBlock = 0;
    currentVisibilityBlock = 0;

    int globalBlockEnd = symbolTable.btab[0].last;
    int curr = globalBlockEnd;

    // 1. Kumpulkan indeks variabel global dan balik urutannya (First-Declared, First-Addressed)
    std::vector<int> globalVars;
    while (curr > globalBlockStart) {
        if (symbolTable.tab[curr].lev == 0 && symbolTable.tab[curr].obj == ObjectType::VARIABLE) {
            globalVars.push_back(curr);
        }
        curr = symbolTable.tab[curr].link;
    }
    std::reverse(globalVars.begin(), globalVars.end());

    // 2. Hitung ukuran dan assign 'adr'
    int totalGlobalMemory = 0;
    int currentOffset = 3; // Laci 0, 1, 2 dipesan untuk Static Link, Dynamic Link, Return Address

    for (int varIndex : globalVars) {
        int size = 0;
        if (symbolTable.tab[varIndex].type == DataType::ARRAY) {
            size = symbolTable.atab[symbolTable.tab[varIndex].ref].size;
        } else if (symbolTable.tab[varIndex].type == DataType::RECORD) {
            size = symbolTable.btab[symbolTable.tab[varIndex].ref].vsze;
        } else {
            size = getDataTypeSize(symbolTable.tab[varIndex].type);
        }
        
        // Assign address untuk variabel ini
        symbolTable.tab[varIndex].adr = currentOffset;
        
        currentOffset += size;
        totalGlobalMemory += size;
    }
    
    // Simpan ukuran total memori variabel global
    symbolTable.btab[0].vsze = totalGlobalMemory;
    symbolTable.btab[0].psze = 0;
    symbolTable.btab[0].lpar = 0; 

    if (node->statements) {
        node->statements->accept(this);
    }
    node->scopeLevel = currentLevel;
}

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

void SemanticVisitor::visit(VarDeclarationNode *node)
{
    if (!node) return;
    node->scopeLevel = currentLevel;

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
        node->tabIndex = index; 
        
        symbolTable.tab[index].nrm = node->isReference ? 0 : 1;
    } else {
        reportError(node, "Duplicate identifier: " + node->name);
    }
}

void SemanticVisitor::visit(ConstDeclarationNode *node)
{
    if (!node) return;
    node->scopeLevel = currentLevel;

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
        if (auto intLit = std::dynamic_pointer_cast<IntegerLiteralNode>(node->value)) {
            symbolTable.tab[index].adr = intLit->value;
        }
        node->tabIndex = index;
    }
}

void SemanticVisitor::visit(TypeDeclarationNode *node)
{
    if (!node) return;
    node->scopeLevel = currentLevel;

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
        node->tabIndex = index;
    }
}

void SemanticVisitor::visit(ProcedureDeclarationNode *node)
{
    if (!node) return;
    node->scopeLevel = currentLevel;

    // Tambahkan prosedur ke scope parent
    int procIndex = symbolTable.enter(node->name, ObjectType::PROCEDURE, DataType::UNKNOWN, currentLevel);
    if (procIndex == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
        return;
    } else {
        node->tabIndex = procIndex;
    }

    // Siapkan Block baru untuk Prosedur
    BtabEntry procBlock;
    procBlock.last = procIndex;
    procBlock.lpar = -1;
    procBlock.psze = 0;
    procBlock.vsze = 0;
    symbolTable.btab.push_back(procBlock);
    symbolTable.tab[procIndex].ref = symbolTable.btab.size() - 1;

    // Pindah masuk ke Scope Prosedur
    currentLevel++;
    int prevBlock = symbolTable.currentBlock;
    int prevVisibility = currentVisibilityBlock;

    symbolTable.currentBlock = symbolTable.btab.size() - 1;
    currentVisibilityBlock = symbolTable.currentBlock;

    int initialLastIndex = procIndex; // Gunakan procIndex langsung sebagai batas aman

    // VISIT PARAMETER (Cukup SATU kali saja, duplikatnya sudah dihapus)
    for (auto &param : node->parameters) { 
        param->accept(this); 
    }

    // Hitung Parameter Size
    int paramLastIndex = symbolTable.btab[symbolTable.currentBlock].last;
    int currParam = paramLastIndex;
    
    std::vector<int> paramVars;
    while (currParam > 0 && currParam != initialLastIndex) { 
        if (symbolTable.tab[currParam].obj == ObjectType::VARIABLE) {
            paramVars.push_back(currParam);
        }
        currParam = symbolTable.tab[currParam].link;
    }
    std::reverse(paramVars.begin(), paramVars.end());

    int totalParamMemory = 0;
    int currentOffset = 3; // Parameter mulai di laci 3

    for (int pIndex : paramVars) {
        int size = 0;

        // Alokasi Pointer
        if (symbolTable.tab[pIndex].nrm == 0) {
            size = 1; 
        } 
        else {
            // Jika nrm == 1 (Pass by Value biasa), hitung ukuran riilnya
            if (symbolTable.tab[pIndex].type == DataType::ARRAY) {
                size = symbolTable.atab[symbolTable.tab[pIndex].ref].size;
            } else if (symbolTable.tab[pIndex].type == DataType::RECORD) {
                size = symbolTable.btab[symbolTable.tab[pIndex].ref].vsze;
            } else {
                size = getDataTypeSize(symbolTable.tab[pIndex].type);
            }
        }
        
        symbolTable.tab[pIndex].adr = currentOffset;
        currentOffset += size;
        totalParamMemory += size;
    }

    symbolTable.btab[symbolTable.currentBlock].lpar = paramLastIndex;
    symbolTable.btab[symbolTable.currentBlock].psze = totalParamMemory;

    // Visit Body (Variables & Statements)
    if (node->body) { 
        node->body->accept(this); 
    }

    // Hitung Local Variable Size
    int finalLastIndex = symbolTable.btab[symbolTable.currentBlock].last;
    int currLocal = finalLastIndex;

    std::vector<int> localVars;
    while (currLocal > 0 && currLocal != paramLastIndex) { 
        if (symbolTable.tab[currLocal].obj == ObjectType::VARIABLE) {
            localVars.push_back(currLocal);
        }
        currLocal = symbolTable.tab[currLocal].link;
    }
    std::reverse(localVars.begin(), localVars.end());

    int totalLocalMemory = 0;

    for (int lIndex : localVars) {
        int size = 0;
        if (symbolTable.tab[lIndex].type == DataType::ARRAY) {
            size = symbolTable.atab[symbolTable.tab[lIndex].ref].size; 
        } else if (symbolTable.tab[lIndex].type == DataType::RECORD) {
            size = symbolTable.btab[symbolTable.tab[lIndex].ref].vsze;
        } else {
            size = getDataTypeSize(symbolTable.tab[lIndex].type);
        }
        symbolTable.tab[lIndex].adr = currentOffset;
        currentOffset += size;
        totalLocalMemory += size;
    }

    node->localVariablesSize = totalLocalMemory;
    symbolTable.btab[symbolTable.currentBlock].vsze = totalLocalMemory;

    // Keluar dari Scope Prosedur
    symbolTable.currentBlock = prevBlock;
    currentVisibilityBlock = prevVisibility;
    currentLevel--;
}

void SemanticVisitor::visit(FunctionDeclarationNode *node)
{
    if (!node) return;
    node->scopeLevel = currentLevel;

    node->returnType->accept(this);
    DataType returnType = node->returnType->resolvedType;
    
    // Tambahkan fungsi ke scope parent
    int funcIndex = symbolTable.enter(node->name, ObjectType::FUNCTION, returnType, currentLevel);
    if (funcIndex == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
        return;
    } else {
        node->tabIndex = funcIndex;
    }

    node->scopeLevel = currentLevel;

    // Siapkan block baru untuk Fungsi
    BtabEntry funcBlock;
    funcBlock.last = funcIndex;
    funcBlock.lpar = -1;
    funcBlock.psze = 0;
    funcBlock.vsze = 0;
    symbolTable.btab.push_back(funcBlock);
    symbolTable.tab[funcIndex].ref = symbolTable.btab.size() - 1;

    // Pindah masuk ke Scope Fungsi
    currentLevel++;
    int prevBlock = symbolTable.currentBlock;
    int prevVisibility = currentVisibilityBlock;

    symbolTable.currentBlock = symbolTable.btab.size() - 1;
    currentVisibilityBlock = symbolTable.currentBlock;

    // Register function name sebagai local variable (untuk menampung Return Value)
    int nameIndex = symbolTable.enter(node->name, ObjectType::VARIABLE, returnType, currentLevel);
    if (nameIndex == -1) {
        reportError(node, "Failed to register function name in local scope: " + node->name);
    }
    else {
        symbolTable.tab[nameIndex].nrm = 1;
    }

    int currentOffset = 3;
    
    // Assign adr untuk variabel nama fungsi (Laci 3)
    if (nameIndex != -1) {
        int nameSize = 0;
        if (symbolTable.tab[nameIndex].type == DataType::ARRAY) {
            nameSize = symbolTable.atab[symbolTable.tab[nameIndex].ref].size;
        } else if (symbolTable.tab[nameIndex].type == DataType::RECORD) {
            nameSize = symbolTable.btab[symbolTable.tab[nameIndex].ref].vsze;
        } else {
            nameSize = getDataTypeSize(symbolTable.tab[nameIndex].type);
        }
        symbolTable.tab[nameIndex].adr = currentOffset;
        currentOffset += nameSize;
    }

    int initialLastIndex = symbolTable.btab[symbolTable.currentBlock].last;

    // Visit Parameter
    for (auto &param : node->parameters) { 
        param->accept(this); 
    }

    // Hitung Parameter size
    int paramLastIndex = symbolTable.btab[symbolTable.currentBlock].last;
    int currParam = paramLastIndex;

    std::vector<int> paramVars;
    while (currParam > initialLastIndex) {
        if (symbolTable.tab[currParam].obj == ObjectType::VARIABLE) {
            paramVars.push_back(currParam);
        }
        currParam = symbolTable.tab[currParam].link;
    }
    std::reverse(paramVars.begin(), paramVars.end());

    int totalParamMemory = 0;

    for (int pIndex : paramVars) {
        int size = 0;
        if (symbolTable.tab[pIndex].nrm == 0) {
            size = 1;
        } else if (symbolTable.tab[pIndex].type == DataType::ARRAY) {
            size = symbolTable.atab[symbolTable.tab[pIndex].ref].size;
        } else if (symbolTable.tab[pIndex].type == DataType::RECORD) {
            size = symbolTable.btab[symbolTable.tab[pIndex].ref].vsze;
        } else {
            size = getDataTypeSize(symbolTable.tab[pIndex].type);
        }
        symbolTable.tab[pIndex].adr = currentOffset;
        currentOffset += size;
        totalParamMemory += size;
    }
    
    symbolTable.btab[symbolTable.currentBlock].lpar = paramLastIndex;
    symbolTable.btab[symbolTable.currentBlock].psze = totalParamMemory;

    // Visit Body
    if (node->body) { 
        node->body->accept(this); 
    }

    // Hitung Local Variable Size
    int finalLastIndex = symbolTable.btab[symbolTable.currentBlock].last;
    int currLocal = finalLastIndex;

    std::vector<int> localVars;
    while (currLocal > paramLastIndex) {
        if (symbolTable.tab[currLocal].obj == ObjectType::VARIABLE) {
            localVars.push_back(currLocal);
        }
        currLocal = symbolTable.tab[currLocal].link;
    }
    std::reverse(localVars.begin(), localVars.end());

    int totalLocalMemory = 0;

    for (int lIndex : localVars) {
        int size = 0;
        if (symbolTable.tab[lIndex].type == DataType::ARRAY) {
            size = symbolTable.atab[symbolTable.tab[lIndex].ref].size;
        } else if (symbolTable.tab[lIndex].type == DataType::RECORD) {
            size = symbolTable.btab[symbolTable.tab[lIndex].ref].vsze;
        } else {
            size = getDataTypeSize(symbolTable.tab[lIndex].type);
        }
        symbolTable.tab[lIndex].adr = currentOffset;
        currentOffset += size;
        totalLocalMemory += size;
    }

    node->localVariablesSize = totalLocalMemory;
    symbolTable.btab[symbolTable.currentBlock].vsze = totalLocalMemory;

    // Exit scope
    symbolTable.currentBlock = prevBlock;
    currentVisibilityBlock = prevVisibility;
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
        int index = symbolTable.lookup(node->name, symbolTable.btab[currentVisibilityBlock].last);
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
if (!node || node->resolvedType != DataType::UNKNOWN) return;

    int elementAtabRef = 0;
    DataType indexType = DataType::UNKNOWN; DataType elementType = DataType::UNKNOWN;
    int low = 0, high = 0, elementSize = 0;

    auto getBoundValue = [&](std::shared_ptr<ExpressionNode> boundNode) -> int {
        if (!boundNode) return 0;
        if (auto intLit = std::dynamic_pointer_cast<IntegerLiteralNode>(boundNode)) {
            return intLit->value;
        }
        if (auto varAcc = std::dynamic_pointer_cast<VarAccessNode>(boundNode)) {
            int idx = symbolTable.lookup(varAcc->name, symbolTable.btab[currentVisibilityBlock].last);
            if (idx != 0 && symbolTable.tab[idx].obj == ObjectType::CONSTANT) {
                return symbolTable.tab[idx].adr;
            }
        }
        return 0;
    };

    if (node->indexType) {
        node->indexType->accept(this);
        if (auto rangeType = std::dynamic_pointer_cast<RangeTypeNode>(node->indexType)) {
            if (rangeType->lowBound) {
                indexType = rangeType->lowBound->evaluatedType;
                low = getBoundValue(rangeType->lowBound);
            }
            if (rangeType->highBound) {
                high = getBoundValue(rangeType->highBound);
            }
        } else {
            indexType = node->indexType->resolvedType;
        }
    }

    if (node->elementType) {
        node->elementType->accept(this);
        elementType = node->elementType->resolvedType;
        
        if (auto simpleType = std::dynamic_pointer_cast<SimpleTypeNode>(node->elementType)) {
            // Gunakan currentVisibilityBlock untuk mencari tipe elemen
            int typeIndex = symbolTable.lookup(simpleType->name, symbolTable.btab[currentVisibilityBlock].last);
            if (typeIndex != 0 && symbolTable.tab[typeIndex].obj == ObjectType::TYPE) {
                elementType = symbolTable.tab[typeIndex].type;
            }
        }
        
        if (elementType == DataType::ARRAY) {
            elementSize = symbolTable.atab[node->elementType->resolvedRef].size;
        } else if (elementType == DataType::RECORD) {
            elementSize = symbolTable.btab[node->elementType->resolvedRef].vsze;
        } else {
            elementSize = getDataTypeSize(elementType);
        }
        elementAtabRef = node->elementType->resolvedRef; 
    }

    AtabEntry arrayEntry;
    arrayEntry.xtyp = indexType; arrayEntry.etyp = elementType; arrayEntry.eref = elementAtabRef;
    arrayEntry.low = low; arrayEntry.high = high; arrayEntry.elsz = elementSize;
    arrayEntry.size = (high - low + 1) * elementSize;
    
    symbolTable.atab.push_back(arrayEntry);
    
    node->resolvedType = DataType::ARRAY; 
    node->resolvedRef = symbolTable.atab.size() - 1;
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
    node->resolvedType = DataType::RECORD;
    node->resolvedRef = symbolTable.currentBlock;
    symbolTable.currentBlock = prevBlock;
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

    DataType lowType = DataType::UNKNOWN;
    DataType highType = DataType::UNKNOWN;

    if (node->lowBound) {
        node->lowBound->accept(this);
        lowType = node->lowBound->evaluatedType; 
    }
    if (node->highBound) {
        node->highBound->accept(this);
        highType = node->highBound->evaluatedType;
    }

    if (lowType != DataType::UNKNOWN && highType != DataType::UNKNOWN) {
        if (lowType != highType) {
            reportError(node, "Type compatibility error: Range bounds must have the exact same type.");
        }
        else if (lowType != DataType::INTEGER && lowType != DataType::CHAR && lowType != DataType::ENUMERATED) {
            reportError(node, "Type error: Range bounds must be of ordinal type (Integer, Char, or Enumerated).");
        }
    }

    node->baseType = lowType; 
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
    node->scopeLevel = currentLevel;

    // Look up variable in symbol table
    int index = symbolTable.lookup(node->name, symbolTable.btab[currentVisibilityBlock].last);
    if (index == 0) {
        reportError(node, "Undefined identifier: " + node->name);
        node->evaluatedType = DataType::UNKNOWN;
    }
    else {
        ObjectType objKind = symbolTable.tab[index].obj;
        if (objKind == ObjectType::PROCEDURE || objKind == ObjectType::TYPE) {
            reportError(node, "Invalid access: '" + node->name + "' is a " + 
                              (objKind == ObjectType::TYPE ? "type" : "procedure") + 
                              ", not a variable or constant");
            node->evaluatedType = DataType::UNKNOWN;
            return;
        }
        node->evaluatedType = symbolTable.tab[index].type;
        node->evaluatedRef = symbolTable.tab[index].ref;
        node->tabIndex = index;
        // Check constant
        if (symbolTable.tab[index].obj == ObjectType::CONSTANT) {
            node->isConstant = true;
        }
    }
}

void SemanticVisitor::visit(ArrayAccessNode *node)
{
    if (!node) return;
    node->scopeLevel = currentLevel;

    DataType expectedIndexType = DataType::UNKNOWN;

    if (node->target) {
        node->target->accept(this);

        if (node->target->evaluatedType != DataType::ARRAY) {
            reportError(node, "Type mismatch: Target variable is not an array");
            node->evaluatedType = DataType::UNKNOWN;
            return;
        }
        
        int atabIndex = node->target->evaluatedRef;
        if (atabIndex != 0) {
            // Evaluasi tipe elemen (etyp)
            node->evaluatedType = symbolTable.atab[atabIndex].etyp;
            node->evaluatedRef = symbolTable.atab[atabIndex].eref;
            // Simpan tipe indeks yang diizinkan untuk array ini (xtyp)
            expectedIndexType = symbolTable.atab[atabIndex].xtyp;
        } else {
            node->evaluatedType = DataType::UNKNOWN;
        }
    }

    if (node->index) {
        node->index->accept(this);
        DataType passedIndexType = node->index->evaluatedType;

        // Bandingkan indeks yang dipakai dengan deklarasi xtyp di atab
        if (expectedIndexType != DataType::UNKNOWN) {
            if (!isCompatible(expectedIndexType, 0, passedIndexType, node->index->evaluatedRef)) {
                reportError(node, "Array index type mismatch");
            }
        }
    }
}

void SemanticVisitor::visit(FieldAccessNode *node)
{
    if (!node) return;
    node->scopeLevel = currentLevel;

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
        node->tabIndex = index;
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
    if (!node || !node->operand) return;

    // Evaluasi operand
    node->operand->accept(this);
    DataType opType = node->operand->evaluatedType;
    
    if (opType == DataType::SUBRANGE) {
        opType = DataType::INTEGER;
    }

    std::string lowerOp = node->op;
    std::transform(lowerOp.begin(), lowerOp.end(), lowerOp.begin(), ::tolower);

    if (lowerOp == "not") { 
        if (opType != DataType::BOOLEAN) {
            reportError(node, "Type mismatch: 'not' operator requires boolean operand");
        }
        node->evaluatedType = DataType::BOOLEAN;
    } 
    else if (lowerOp == "-" || lowerOp == "+") {
        if (opType != DataType::INTEGER && opType != DataType::REAL) {
            reportError(node, "Type mismatch: unary sign operator requires numeric operand (Integer/Real)");
        }
        node->evaluatedType = opType;
    } 
    else {
        node->evaluatedType = DataType::UNKNOWN;
    }
}

void SemanticVisitor::visit(FunctionCallNode *node)
{
    if (!node) return;
    node->scopeLevel = currentLevel;

    int searchLimit = symbolTable.btab[currentVisibilityBlock].last;
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
    int blockIdx = symbolTable.tab[index].ref;
    std::vector<int> expectedParams;
    int currParam = symbolTable.btab[blockIdx].lpar;
    while (currParam > index + 1) {
        expectedParams.push_back(currParam);
        currParam = symbolTable.tab[currParam].link;
    }
    std::reverse(expectedParams.begin(), expectedParams.end());
    if (node->args.size() != expectedParams.size()) {
        reportError(node, "Parameter count mismatch for '" + node->name + 
                          "'. Expected " + std::to_string(expectedParams.size()) + 
                          ", but got " + std::to_string(node->args.size()));
    } 
    else {
        for (size_t k = 0; k < node->args.size(); ++k) {
            if (node->args[k]) {
                node->args[k]->accept(this);
                int pIdx = expectedParams[k];
                DataType paramType = symbolTable.tab[pIdx].type;
                int paramRef = symbolTable.tab[pIdx].ref;
                if (!isCompatible(paramType, paramRef, node->args[k]->evaluatedType, node->args[k]->evaluatedRef)) {
                    reportError(node->args[k].get(), "Type mismatch in argument " + std::to_string(k + 1) + 
                                                     " for procedure '" + node->name + "'");
                }
            }
        }
    }

    // Function return type
    node->evaluatedType = symbolTable.tab[index].type;
    node->tabIndex = index;
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

    ASTNode *errorNode = node;
    if (node->target) {
        errorNode = node->target.get();
    }

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
                reportError(errorNode, "Illegal assignment: '" + varAccess->name + "' is a constant and cannot be modified");
            }
            // Loop counter check
            if (std::find(activeLoopCounters.begin(), activeLoopCounters.end(), varAccess->name) != activeLoopCounters.end()) {
                reportError(errorNode, "Illegal assignment: FOR loop counter '" + varAccess->name + "' cannot be modified");
            }
        }

        // Type compatibility check
        if (!isCompatible(node->target->evaluatedType, node->target->evaluatedRef, node->value->evaluatedType, node->value->evaluatedRef)) {
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
                    if (!isCompatible(exprType, node->expression->evaluatedRef, constType, caseConst->evaluatedRef)) {
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
    int index = symbolTable.lookup(node->counterVar, symbolTable.btab[currentVisibilityBlock].last);

    if (index == 0) {
        reportError(node, "Undefined loop variable: " + node->counterVar);
    }
    else if (symbolTable.tab[index].type != DataType::INTEGER) {
        reportError(node, "For loop counter must be integer");
        node->counterTabIndex = index;
    }
    else {
        node->counterTabIndex = index;
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
    node->scopeLevel = currentLevel;

    std::string lowerName = node->name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    int searchLimit = symbolTable.btab[currentVisibilityBlock].last;
    int index = 0;

    int i = searchLimit;
    while (i > 0) {
        std::string tableName = symbolTable.tab[i].name;
        std::transform(tableName.begin(), tableName.end(), tableName.begin(), ::tolower);

        if (tableName == lowerName && symbolTable.tab[i].obj == ObjectType::PROCEDURE) {
            index = i;
            break;
        }
        i = symbolTable.tab[i].link;
    }

    if (index == 0) {
        reportError(node, "Undefined procedure: " + node->name);
        return;
    }

    node->tabIndex = index;

    if (lowerName == "write" || lowerName == "writeln") {
        for (auto& arg : node->args) {
            if (arg) {
                arg->accept(this);
            }
        }
        return;
    }
    if (lowerName == "read" || lowerName == "readln") {
        for (auto& arg : node->args) {
            if (arg) {
                arg->accept(this);
                auto varAccess = std::dynamic_pointer_cast<VarAccessNode>(arg);
                
                if (!varAccess) {
                    reportError(arg.get(), "Semantic Error: Argument for '" + lowerName + "' must be a variable.");
                } else if (varAccess->isConstant) {
                    reportError(arg.get(), "Semantic Error: Cannot read into constant '" + varAccess->name + "'.");
                }
            }
        }
        return;
    }

    int blockIdx = symbolTable.tab[index].ref;
    std::vector<int> expectedParams;
    int currParam = symbolTable.btab[blockIdx].last;
    while (currParam > index) {
        // FILTER: Kita hanya ingin mengambil variabel yang bertindak sebagai PARAMETER.
        // periksa apakah index-nya berada dalam rentang parameter formal.
        // Karena parameter didaftarkan duluan, indeksnya pasti <= lpar (Last Parameter)
        if (symbolTable.tab[currParam].obj == ObjectType::VARIABLE && 
            currParam <= symbolTable.btab[blockIdx].lpar) {
            expectedParams.push_back(currParam);
        }
        currParam = symbolTable.tab[currParam].link;
    }
    std::reverse(expectedParams.begin(), expectedParams.end());
    if (node->args.size() != expectedParams.size()) {
        reportError(node, "Parameter count mismatch for '" + node->name + 
                          "'. Expected " + std::to_string(expectedParams.size()) + 
                          ", but got " + std::to_string(node->args.size()));
    } 
    else {
        for (size_t k = 0; k < node->args.size(); ++k) {
            if (node->args[k]) {
                node->args[k]->accept(this);
                int pIdx = expectedParams[k];

                // Validasi Arugemen Parameter Var
                // Jika targetnya adalah parameter VAR
                if (symbolTable.tab[pIdx].nrm == 0) { 
                    auto varAccess = std::dynamic_pointer_cast<VarAccessNode>(node->args[k]);
                    
                    // Cek 1: Harus berupa variabel access (bukan literal seperti IntLiteral/CharLiteral atau ArrayAccess mentah jika tidak valid)
                    // Catatan: ArrayAccess / FieldAccess juga boleh dioper ke VAR jika menghasilkan L-Value
                    if (!varAccess) {
                        reportError(node->args[k].get(), "Semantic Error: Variable parameter '" + symbolTable.tab[pIdx].name + 
                                                         "' requires a variable reference as an argument.");
                        continue;
                    }
                    
                    // Cek 2: Tidak boleh mengoper CONSTANT ke parameter VAR (karena nilainya bisa dimanipulasi di dalam prosedur)
                    int argTabIdx = varAccess->tabIndex;
                    if (symbolTable.tab[argTabIdx].obj == ObjectType::CONSTANT) {
                        reportError(node->args[k].get(), "Semantic Error: Cannot pass constant '" + varAccess->name + 
                                                         "' as a VAR parameter.");
                        continue;
                    }
                }


                DataType paramType = symbolTable.tab[pIdx].type;
                int paramRef = symbolTable.tab[pIdx].ref;
                if (!isCompatible(paramType, paramRef, node->args[k]->evaluatedType, node->args[k]->evaluatedRef)) {
                    reportError(node->args[k].get(), "Type mismatch in argument " + std::to_string(k + 1) + 
                                                     " for procedure '" + node->name + "'");
                }
            }
        }
    }
}

void SemanticVisitor::visit(EmptyStatementNode *node)
{
    if (!node) return;
    // Empty statement does nothing
}

// ============= HELPER METHODS =============

bool SemanticVisitor::isCompatible(DataType target, int targetRef, DataType source, int sourceRef)
{
    if (target == source) {
        if (target == DataType::ARRAY) {
            if (targetRef != 0 && sourceRef != 0) {
                bool isSameSize = symbolTable.atab[targetRef].size == symbolTable.atab[sourceRef].size;
                bool isSameElementType = symbolTable.atab[targetRef].etyp == symbolTable.atab[sourceRef].etyp;
                return isSameSize && isSameElementType;
            }
        }
        if (target == DataType::RECORD) {
            return targetRef == sourceRef; 
        }
        return true; 
    }

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
    
    if (left == DataType::SUBRANGE) left = DataType::INTEGER;
    if (right == DataType::SUBRANGE) right = DataType::INTEGER;

    // mod and div only valid for integer
    if (lowerOp == "mod" || lowerOp == "div") {
        if (left == DataType::INTEGER && right == DataType::INTEGER) {
            return DataType::INTEGER;
        }
        return DataType::UNKNOWN; 
    }

    // Arithmetic operators: +, -, *, /
    if (lowerOp == "+" || lowerOp == "-" || lowerOp == "*" || lowerOp == "/") {
        if ((left == DataType::INTEGER || left == DataType::REAL) &&
            (right == DataType::INTEGER || right == DataType::REAL)) {
            // / always real
            if (lowerOp == "/") return DataType::REAL; 
            // Result is REAL if either operand is REAL, otherwise INTEGER
            return (left == DataType::REAL || right == DataType::REAL) ? DataType::REAL : DataType::INTEGER;
        }
    }

    // Comparison operators: ==, <>, <, <=, >, >=
    if (lowerOp == "==" || lowerOp == "=" || lowerOp == "<>" || lowerOp == "<" || lowerOp == "<=" || lowerOp == ">" || lowerOp == ">=") {
        // only if same data type
        if (isCompatible(left, 0, right, 0) || isCompatible(right, 0, left, 0)) {
            return DataType::BOOLEAN;
        }
        return DataType::UNKNOWN;
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
            return 1;
        case DataType::REAL:
            return 1;
        case DataType::BOOLEAN:
            return 1;
        case DataType::CHAR:
            return 1;
        case DataType::STRING:
            return 1;
        case DataType::SUBRANGE:
            return 1;
        case DataType::ENUMERATED:
            return 1;
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
