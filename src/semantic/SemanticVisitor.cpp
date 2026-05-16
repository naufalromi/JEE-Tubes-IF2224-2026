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

    // Create or initialize btab[0] for global program block
    int globalBlockStart = symbolTable.btab.back().last;

    // Process Declaration
    for (auto &decl : node->declarations) {
        decl->accept(this);
    }

    // Calculate variable count for global block (vsze)
    int globalBlockEnd = symbolTable.btab.back().last;
    int varCount = 0;
    // Count variables in global scope (skip reserved words and types)
    for (int i = globalBlockStart + 1; i <= globalBlockEnd; i++) {
        if (symbolTable.tab[i].lev == 0 && symbolTable.tab[i].obj == ObjectType::VARIABLE) {
            varCount++;
        }
    }
    symbolTable.btab[0].vsze = varCount;

    // Create block table entry for main program block (compound statement)
    if (symbolTable.btab.size() == 1) {
        BtabEntry mainBlock;
        mainBlock.last = symbolTable.btab.back().last;
        mainBlock.lpar = -1;
        mainBlock.psze = 0;
        mainBlock.vsze = 0;  // Main block has no local variables (they're in global scope)
        symbolTable.btab.push_back(mainBlock);
    }

    // Process Compound Statement (main block)
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

    if (index == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
    }
    else {
        node->scopeLevel = currentLevel;
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

    // For main program block (level 0), create btab entry
    if (currentLevel == 0 && symbolTable.btab.size() == 1) {
        // This is the main block of the program
        BtabEntry mainBlock;
        mainBlock.last = symbolTable.btab.back().last;
        mainBlock.lpar = -1;
        mainBlock.psze = 0;
        mainBlock.vsze = 0;
        symbolTable.btab.push_back(mainBlock);
    }

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
    }
}

void SemanticVisitor::visit(ProcedureDeclarationNode *node)
{
    if (!node) return;

    // Enter procedure into symbol table at current level
    int procIndex = symbolTable.enter(node->name, ObjectType::PROCEDURE, DataType::UNKNOWN, currentLevel);

    if (procIndex == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
        return;
    }

    node->scopeLevel = currentLevel;

    // Enter new scope for procedure body
    currentLevel++;

    // Create block table entry
    BtabEntry procBlock;
    procBlock.last = symbolTable.tab.size() - 1;
    procBlock.lpar = -1;
    procBlock.psze = 0;
    procBlock.vsze = 0;
    symbolTable.btab.push_back(procBlock);

    // Process parameters (add to symbol table)
    for (auto &param : node->parameters) {
        param->accept(this);
    }

    // Process procedure body
    if (node->body) {
        node->body->accept(this);
    }

    // Exit scope
    symbolTable.btab.pop_back();
    currentLevel--;
}

void SemanticVisitor::visit(FunctionDeclarationNode *node)
{
    if (!node) return;

    // Process return type
    node->returnType->accept(this);

    // Enter function into symbol table at current level
    DataType returnType = node->returnType->resolvedType;
    int funcIndex = symbolTable.enter(node->name, ObjectType::FUNCTION, returnType, currentLevel);

    if (funcIndex == -1) {
        reportError(node, "Duplicate identifier: " + node->name);
        return;
    }

    node->scopeLevel = currentLevel;

    // Enter new scope for function body
    currentLevel++;

    // Create block table entry
    BtabEntry funcBlock;
    funcBlock.last = symbolTable.tab.size() - 1;
    funcBlock.lpar = -1;
    funcBlock.psze = 0;
    funcBlock.vsze = 0;
    symbolTable.btab.push_back(funcBlock);

    // Register function name as a variable in function scope (for return value assignment)
    // This allows: Faktorial := value inside the function body
    int nameIndex = symbolTable.enter(node->name, ObjectType::VARIABLE, returnType, currentLevel);
    if (nameIndex == -1) {
        reportError(node, "Failed to register function name in local scope: " + node->name);
    }

    // Process parameters
    for (auto &param : node->parameters) {
        param->accept(this);
    }

    // Process function body
    if (node->body) {
        node->body->accept(this);
    }

    // Exit scope
    symbolTable.btab.pop_back();
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
        int index = symbolTable.lookup(node->name, symbolTable.btab.back().last);
        if (index != 0) {
            node->resolvedType = symbolTable.tab[index].type;
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
        
        // Check element type and calculate size accordingly
        if (auto rangeType = std::dynamic_pointer_cast<RangeTypeNode>(node->elementType)) {
            // Element is a range/subrange type - calculate size from base type
            if (rangeType->lowBound) {
                DataType elemBase = rangeType->lowBound->evaluatedType;
                elementSize = getDataTypeSize(elemBase);
            }
        } else if (auto simpleType = std::dynamic_pointer_cast<SimpleTypeNode>(node->elementType)) {
            // Element is a user-defined type - look it up in symbol table
            int typeIndex = symbolTable.lookup(simpleType->name, symbolTable.btab.back().last);
            if (typeIndex != 0 && symbolTable.tab[typeIndex].obj == ObjectType::TYPE) {
                // User-defined type - store its symbol table index as reference
                elementAtabRef = typeIndex;
                // Estimate size for composite types
                if (elementType == DataType::RECORD) {
                    elementSize = 256;  // Estimated record size
                } else {
                    elementSize = getDataTypeSize(elementType);
                }
            } else {
                elementSize = getDataTypeSize(elementType);
            }
        } else {
            // Built-in simple types
            elementSize = getDataTypeSize(elementType);
        }
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
    
    node->resolvedType = elementType;
}

void SemanticVisitor::visit(RecordTypeNode *node)
{
    if (!node) return;

    // Process all fields in record
    for (auto &field : node->fields) {
        field->accept(this);
    }

    node->resolvedType = DataType::RECORD;
}

void SemanticVisitor::visit(EnumeratedTypeNode *node)
{
    if (!node) return;
    node->resolvedType = DataType::ENUMERATED;
}

void SemanticVisitor::visit(RangeTypeNode *node)
{
    if (!node) return;

    DataType boundType = DataType::UNKNOWN;

    // Process bounds - just evaluate them, don't create atab entry yet
    if (node->lowBound) {
        node->lowBound->accept(this);
        boundType = node->lowBound->evaluatedType;  // Get base type (INTEGER, CHAR, etc.)
    }

    if (node->highBound) {
        node->highBound->accept(this);
    }

    // Store the base type (used by ArrayTypeNode to create atab entry)
    // resolvedType is SUBRANGE (semantic marker), but ArrayTypeNode will extract bounds from lowBound/highBound
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
    int index = symbolTable.lookup(node->name, symbolTable.btab.back().last);
    if (index == 0) {
        reportError(node, "Undefined identifier: " + node->name);
        node->evaluatedType = DataType::UNKNOWN;
    }
    else {
        node->evaluatedType = symbolTable.tab[index].type;
    }
}

void SemanticVisitor::visit(ArrayAccessNode *node)
{
    if (!node) return;

    // Check target is array
    if (node->target) {
        node->target->accept(this);
        // Type of array access is the element type
        node->evaluatedType = node->target->evaluatedType;
    }

    // Check index is valid
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

    // Check target is record
    if (node->target) {
        node->target->accept(this);
        if (node->target->evaluatedType != DataType::RECORD) {
            reportError(node, "Can only access fields of record type");
        }
    }

    // Field type would need record structure info
    node->evaluatedType = DataType::UNKNOWN;
}

void SemanticVisitor::visit(BinaryOpNode *node)
{
    if (!node) return;

    // Evaluate both operands
    if (node->left) {
        node->left->accept(this);
    }
    if (node->right) {
        node->right->accept(this);
    }

    // Type checking based on operator
    node->evaluatedType = resolveBinaryType(node->op,
                                            node->left->evaluatedType,
                                            node->right->evaluatedType);
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

    // Look up function - search specifically for FUNCTION entries
    int index = 0;
    int searchLimit = symbolTable.btab.back().last;
    
    // Search backwards from current scope to find FUNCTION entry
    for (int i = searchLimit; i >= 0; i--) {
        if (symbolTable.tab[i].name == node->name && symbolTable.tab[i].obj == ObjectType::FUNCTION) {
            index = i;
            break;
        }
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

    // Evaluate case expression
    if (node->expression) {
        node->expression->accept(this);
    }

    // Process each case
    for (auto &caseItem : node->cases) {
        // Check constants
        for (auto &caseConst : caseItem.first) {
            if (caseConst) {
                caseConst->accept(this);
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
    int index = symbolTable.lookup(node->counterVar, symbolTable.btab.back().last);
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

    // Execute loop body
    if (node->body) {
        node->body->accept(this);
    }
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

    // Look up procedure - search specifically for PROCEDURE entries
    int index = 0;
    int searchLimit = symbolTable.btab.back().last;
    
    // Search backwards from current scope to find PROCEDURE entry
    for (int i = searchLimit; i >= 0; i--) {
        if (symbolTable.tab[i].name == node->name && symbolTable.tab[i].obj == ObjectType::PROCEDURE) {
            index = i;
            break;
        }
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

    return false;
}

DataType SemanticVisitor::resolveBinaryType(const std::string &op, DataType left, DataType right)
{
    // Arithmetic operators: +, -, *, /, mod, div
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "mod" || op == "div") {
        if ((left == DataType::INTEGER || left == DataType::REAL) &&
            (right == DataType::INTEGER || right == DataType::REAL)) {
            // Result is REAL if either operand is REAL, otherwise INTEGER
            return (left == DataType::REAL || right == DataType::REAL) ? DataType::REAL : DataType::INTEGER;
        }
    }

    // Comparison operators: ==, <>, <, <=, >, >=
    if (op == "==" || op == "=" || op == "<>" || op == "<" || op == "<=" || op == ">" || op == ">=") {
        return DataType::BOOLEAN;
    }

    // Logical operators: and, or
    if (op == "and" || op == "or") {
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
