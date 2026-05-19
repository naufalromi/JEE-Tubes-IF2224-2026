#include "ASTPrinter.hpp"
#include <iostream>

static std::string typeToString(DataType t) {
    switch (t) {
        case DataType::INTEGER:    return "integer";
        case DataType::REAL:       return "real";
        case DataType::BOOLEAN:    return "boolean";
        case DataType::CHAR:       return "char";
        case DataType::STRING:     return "string";
        case DataType::RECORD:     return "record";
        case DataType::ARRAY:      return "array";
        case DataType::SUBRANGE:   return "subrange";
        case DataType::ENUMERATED: return "enumerated";
        case DataType::VOID:       return "void";
        default:                   return "unknown";
    }
}

void ASTPrinter::print(std::shared_ptr<ProgramNode> root) {
    if (!root) return;
    currentPrefix = "";
    pendingRole = "";
    isLastChild = true;
    isRoot = true;
    root->accept(this);
}

void ASTPrinter::visitNode(const std::string& text) {
    std::string roleStr = pendingRole.empty() ? "" : pendingRole + " ";
    pendingRole = ""; 

    if (isRoot) {
        std::cout << roleStr << text << "\n";
        isRoot = false;
    } else {
        std::cout << currentPrefix << (isLastChild ? " └─ " : " ├─ ") << roleStr << text << "\n";
    }
}

void ASTPrinter::acceptChild(std::shared_ptr<ASTNode> child, bool last, bool parentWasRoot) {
    if (!child) return;

    std::string oldPrefix = currentPrefix;
    bool oldLast = isLastChild;

    if (!parentWasRoot) {
        currentPrefix += (oldLast ? "    " : " │  ");
    }

    isLastChild = last;
    child->accept(this);

    currentPrefix = oldPrefix;
    isLastChild = oldLast;
}

void ASTPrinter::visit(ProgramNode* node) {
    visitNode("ProgramNode(name: '" + node->name + "')");
    
    size_t totalChildren = node->declarations.size() + (node->statements ? 1 : 0);
    size_t currentIdx = 0;

    for (const auto& decl : node->declarations) {
        currentIdx++;
        acceptChild(decl, currentIdx == totalChildren, true);
    }
    if (node->statements) {
        currentIdx++;
        pendingRole = "main:";
        acceptChild(node->statements, currentIdx == totalChildren, true);
    }
}

void ASTPrinter::visit(BlockNode* node) {
    std::string deco = " → block_index:" + std::to_string(node->scopeLevel) + ", lev:" + std::to_string(node->scopeLevel);
    visitNode("Block" + deco);

    size_t totalChildren = node->declarations.size() + (node->statements ? 1 : 0);
    size_t currentIdx = 0;

    for (const auto& decl : node->declarations) {
        currentIdx++;
        acceptChild(decl, currentIdx == totalChildren);
    }
    if (node->statements) {
        currentIdx++;
        pendingRole = "statements:";
        acceptChild(node->statements, currentIdx == totalChildren);
    }
}

void ASTPrinter::visit(ConstDeclarationNode* node) {
    visitNode("ConstDecl('" + node->name + "') [tab_index:" + std::to_string(node->tabIndex) + ", lev:" + std::to_string(node->scopeLevel) + "]");
    if (node->value) {
        pendingRole = "value:";
        acceptChild(node->value, true);
    }
}

void ASTPrinter::visit(VarDeclarationNode* node) {
    std::string typeStr = node->typeDefinition ? typeToString(node->typeDefinition->resolvedType) : "unknown";
    std::string deco = " → tab_index:" + std::to_string(node->tabIndex) + ", type:" + typeStr + ", lev:" + std::to_string(node->scopeLevel);
    visitNode("VarDecl('" + node->name + "')" + deco);
    
    if (node->typeDefinition) {
        pendingRole = "type_def:";
        acceptChild(node->typeDefinition, true);
    }
}

void ASTPrinter::visit(TypeDeclarationNode* node) {
    std::string typeStr = node->typeDefinition ? typeToString(node->typeDefinition->resolvedType) : "unknown";
    std::string deco = " → tab_index:" + std::to_string(node->tabIndex) + ", type:" + typeStr + ", lev:" + std::to_string(node->scopeLevel);
    visitNode("TypeDecl('" + node->name + "')" + deco);
    
    if (node->typeDefinition) {
        pendingRole = "type_def:";
        acceptChild(node->typeDefinition, true);
    }
}

void ASTPrinter::visit(ProcedureDeclarationNode* node) {
    std::string deco = " → tab_index:" + std::to_string(node->tabIndex) + ", lev:" + std::to_string(node->scopeLevel) + ", local_size:" + std::to_string(node->localVariablesSize);
    visitNode("ProcedureDecl('" + node->name + "')" + deco);

    size_t totalChildren = node->parameters.size() + (node->body ? 1 : 0);
    size_t currentIdx = 0;

    for (const auto& param : node->parameters) {
        currentIdx++;
        pendingRole = "param:";
        acceptChild(param, currentIdx == totalChildren);
    }
    if (node->body) {
        currentIdx++;
        pendingRole = "body:";
        acceptChild(node->body, currentIdx == totalChildren);
    }
}

void ASTPrinter::visit(FunctionDeclarationNode* node) {
    std::string retStr = node->returnType ? typeToString(node->returnType->resolvedType) : "unknown";
    std::string deco = " → tab_index:" + std::to_string(node->tabIndex) + ", return_type:" + retStr + ", lev:" + std::to_string(node->scopeLevel);
    visitNode("FunctionDecl('" + node->name + "')" + deco);

    size_t totalChildren = node->parameters.size() + (node->returnType ? 1 : 0) + (node->body ? 1 : 0);
    size_t currentIdx = 0;

    for (const auto& param : node->parameters) {
        currentIdx++;
        pendingRole = "param:";
        acceptChild(param, currentIdx == totalChildren);
    }
    if (node->returnType) {
        currentIdx++;
        pendingRole = "return:";
        acceptChild(node->returnType, currentIdx == totalChildren);
    }
    if (node->body) {
        currentIdx++;
        pendingRole = "body:";
        acceptChild(node->body, currentIdx == totalChildren);
    }
}

void ASTPrinter::visit(SimpleTypeNode* node) {
    visitNode("SimpleTypeNode('" + node->name + "') → resolved_type:" + typeToString(node->resolvedType));
}

void ASTPrinter::visit(ArrayTypeNode* node) {
    visitNode("ArrayTypeNode [atab_ref:" + std::to_string(node->resolvedRef) + "] → resolved_type:" + typeToString(node->resolvedType));
    
    if (node->indexType) {
        pendingRole = "index_type:";
        acceptChild(node->indexType, node->elementType == nullptr);
    }
    if (node->elementType) {
        pendingRole = "element_type:";
        acceptChild(node->elementType, true);
    }
}

void ASTPrinter::visit(RecordTypeNode* node) {
    visitNode("RecordTypeNode [btab_ref:" + std::to_string(node->resolvedRef) + "] → resolved_type:" + typeToString(node->resolvedType));
    for (size_t i = 0; i < node->fields.size(); ++i) {
        pendingRole = "field:";
        acceptChild(node->fields[i], i == node->fields.size() - 1);
    }
}

void ASTPrinter::visit(RangeTypeNode* node) {
    visitNode("RangeTypeNode [base_type:" + typeToString(node->baseType) + "] → resolved_type:" + typeToString(node->resolvedType));
    
    if (node->lowBound) {
        pendingRole = "low:";
        acceptChild(node->lowBound, node->highBound == nullptr);
    }
    if (node->highBound) {
        pendingRole = "high:";
        acceptChild(node->highBound, true);
    }
}

void ASTPrinter::visit(EnumeratedTypeNode* node) {
    std::string elems = "";
    for (size_t i = 0; i < node->elements.size(); ++i) {
        elems += node->elements[i] + (i < node->elements.size() - 1 ? ", " : "");
    }
    visitNode("EnumeratedTypeNode([" + elems + "]) → resolved_type:" + typeToString(node->resolvedType));
}

void ASTPrinter::visit(IntegerLiteralNode* node) {
    visitNode("IntLiteral(" + std::to_string(node->value) + ") → type:" + typeToString(node->evaluatedType));
}

void ASTPrinter::visit(RealLiteralNode* node) {
    visitNode("RealLiteral(" + std::to_string(node->value) + ") → type:" + typeToString(node->evaluatedType));
}

void ASTPrinter::visit(StringLiteralNode* node) {
    visitNode("StringLiteral('" + node->value + "') → type:" + typeToString(node->evaluatedType));
}

void ASTPrinter::visit(BooleanLiteralNode* node) {
    visitNode("BoolLiteral(" + std::string(node->value ? "True" : "False") + ") → type:" + typeToString(node->evaluatedType));
}

void ASTPrinter::visit(CharLiteralNode* node) {
    std::string charStr(1, node->value);
    visitNode("CharLiteral('" + charStr + "') → type:" + typeToString(node->evaluatedType));
}

void ASTPrinter::visit(VarAccessNode* node) {
    std::string constStr = node->isConstant ? ", CONSTANT" : "";
    std::string deco = " → tab_index:" + std::to_string(node->tabIndex) + ", type:" + typeToString(node->evaluatedType) + constStr + ", lev:" + std::to_string(node->scopeLevel);
    visitNode("VarAccess('" + node->name + "')" + deco);
}

void ASTPrinter::visit(UnaryOpNode* node) {
    visitNode("UnaryOp('" + node->op + "') → type:" + typeToString(node->evaluatedType));
    if (node->operand) {
        pendingRole = "operand:";
        acceptChild(node->operand, true);
    }
}

void ASTPrinter::visit(ArrayAccessNode* node) {
    visitNode("ArrayAccess → type:" + typeToString(node->evaluatedType));
    if (node->target) {
        pendingRole = "target:";
        acceptChild(node->target, node->index == nullptr);
    }
    if (node->index) {
        pendingRole = "index:";
        acceptChild(node->index, true);
    }
}

void ASTPrinter::visit(FieldAccessNode* node) {
    std::string deco = " → tab_index:" + std::to_string(node->tabIndex) + ", type:" + typeToString(node->evaluatedType);
    visitNode("FieldAccess('." + node->fieldName + "')" + deco);
    if (node->target) {
        pendingRole = "target:";
        acceptChild(node->target, true);
    }
}

void ASTPrinter::visit(BinaryOpNode* node) {
    visitNode("BinOp('" + node->op + "') → type:" + typeToString(node->evaluatedType));
    if (node->left) {
        pendingRole = "left:";
        acceptChild(node->left, node->right == nullptr);
    }
    if (node->right) {
        pendingRole = "right:";
        acceptChild(node->right, true);
    }
}

void ASTPrinter::visit(FunctionCallNode* node) {
    std::string deco = " → tab_index:" + std::to_string(node->tabIndex) + ", type:" + typeToString(node->evaluatedType);
    visitNode("FunctionCall('" + node->name + "')" + deco);
    
    for (size_t i = 0; i < node->args.size(); ++i) {
        pendingRole = "arg:";
        acceptChild(node->args[i], i == node->args.size() - 1);
    }
}

void ASTPrinter::visit(CompoundStatementNode* node) {
    visitNode("CompoundStatement");
    for (size_t i = 0; i < node->statements.size(); ++i) {
        acceptChild(node->statements[i], i == node->statements.size() - 1);
    }
}

void ASTPrinter::visit(AssignmentStatementNode* node) {
    visitNode("Assign → type:void");
    if (node->target) {
        pendingRole = "target:";
        acceptChild(node->target, node->value == nullptr);
    }
    if (node->value) {
        pendingRole = "value:";
        acceptChild(node->value, true);
    }
}

void ASTPrinter::visit(IfStatementNode* node) {
    visitNode("IfStatement");
    size_t total = (node->condition ? 1 : 0) + (node->thenBranch ? 1 : 0) + (node->elseBranch ? 1 : 0);
    size_t curr = 0;
    
    if (node->condition) { 
        curr++; 
        pendingRole = "condition:"; 
        acceptChild(node->condition, curr == total); 
    }
    if (node->thenBranch) { 
        curr++; 
        pendingRole = "then:"; 
        acceptChild(node->thenBranch, curr == total); 
    }
    if (node->elseBranch) { 
        curr++; 
        pendingRole = "else:"; 
        acceptChild(node->elseBranch, curr == total); 
    }
}

void ASTPrinter::visit(WhileLoopNode* node) {
    visitNode("WhileLoop");
    if (node->condition) {
        pendingRole = "condition:";
        acceptChild(node->condition, node->body == nullptr);
    }
    if (node->body) {
        pendingRole = "body:";
        acceptChild(node->body, true);
    }
}

void ASTPrinter::visit(ForLoopNode* node) {
    std::string direct = node->isDownTo ? "downto" : "to";
    visitNode("ForLoop('" + node->counterVar + "' " + direct + ")");
    
    size_t total = (node->startValue ? 1 : 0) + (node->endValue ? 1 : 0) + (node->body ? 1 : 0);
    size_t curr = 0;
    
    if (node->startValue) { 
        curr++; 
        pendingRole = "start:"; 
        acceptChild(node->startValue, curr == total); 
    }
    if (node->endValue) { 
        curr++; 
        pendingRole = "end:"; 
        acceptChild(node->endValue, curr == total); 
    }
    if (node->body) { 
        curr++; 
        pendingRole = "body:"; 
        acceptChild(node->body, curr == total); 
    }
}

void ASTPrinter::visit(RepeatUntilNode* node) {
    visitNode("RepeatUntil");
    size_t total = node->body.size() + (node->condition ? 1 : 0);
    size_t curr = 0;
    
    for (const auto& stmt : node->body) {
        curr++;
        acceptChild(stmt, curr == total);
    }
    if (node->condition) {
        curr++;
        pendingRole = "until:";
        acceptChild(node->condition, curr == total);
    }
}

void ASTPrinter::visit(CaseStatementNode* node) {
    visitNode("CaseStatement");
    size_t total = (node->expression ? 1 : 0) + node->cases.size() + (node->elseBranch ? 1 : 0);
    size_t curr = 0;
    
    if (node->expression) {
        curr++;
        pendingRole = "match_expr:";
        acceptChild(node->expression, curr == total);
    }
    for (const auto& caseItem : node->cases) {
        curr++;
        if (caseItem.second) {
            pendingRole = "case_block:";
            acceptChild(caseItem.second, curr == total);
        }
    }
    if (node->elseBranch) {
        curr++;
        pendingRole = "else:";
        acceptChild(node->elseBranch, curr == total);
    }
}

void ASTPrinter::visit(ProcedureCallNode* node) {
    std::string deco = " → tab_index:" + std::to_string(node->tabIndex) + ", type:void";
    visitNode("ProcedureCall('" + node->name + "')" + deco);
    
    for (size_t i = 0; i < node->args.size(); ++i) {
        pendingRole = "arg:";
        acceptChild(node->args[i], i == node->args.size() - 1);
    }
}

void ASTPrinter::visit(EmptyStatementNode* node) {
    visitNode("EmptyStatement");
}