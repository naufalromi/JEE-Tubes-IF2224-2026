#include "ASTPrinter.hpp"
#include <iostream>

void ASTPrinter::print(std::shared_ptr<ProgramNode> root) {
    if (!root) return;
    currentPrefix = "";
    isLastChild = true;
    isRoot = true;
    
    root->accept(this);
}

void ASTPrinter::visitNode(const std::string& text) {
    if (isRoot) {
        std::cout << text << "\n";
        isRoot = false;
    } else {
        std::cout << currentPrefix << (isLastChild ? "└── " : "├── ") << text << "\n";
    }
}

void ASTPrinter::acceptChild(std::shared_ptr<ASTNode> child, bool last, bool parentWasRoot) {
    if (!child) return;

    std::string oldPrefix = currentPrefix;
    bool oldLast = isLastChild;

    if (!parentWasRoot) {
        currentPrefix += oldLast ? "    " : "│   ";
    }
    isLastChild = last;

    child->accept(this);

    currentPrefix = oldPrefix;
    isLastChild = oldLast;
}

void ASTPrinter::visit(ProgramNode* node) {
    visitNode("[ProgramNode] name: '" + node->name + "'");
    std::vector<std::shared_ptr<ASTNode>> children;

    for (const auto& decl : node->declarations) {
        if (decl) children.push_back(decl);
    }
    
    if (node->statements) {
        children.push_back(node->statements);
        // for (const auto& stmt : node->statements->statements) {
        //      if (stmt) children.push_back(stmt);
        // }
    }
    
    for (size_t i = 0; i < children.size(); ++i) {
        bool isLast = (i == children.size() - 1);
        acceptChild(children[i], isLast, true); 
    }
}

void ASTPrinter::visit(BlockNode* node) {
    visitNode("[BlockNode]");    
    std::vector<std::shared_ptr<ASTNode>> children;
    
    for (const auto& decl : node->declarations) {
        if (decl) children.push_back(decl);
    }
    
    if (node->statements) {
        children.push_back(node->statements);
    }

    for (size_t i = 0; i < children.size(); ++i) {
        bool isLast = (i == children.size() - 1);
        acceptChild(children[i], isLast); 
    }
}

void ASTPrinter::visit(ConstDeclarationNode* node) {
    visitNode("[ConstDecl] name: '" + node->name + "'");
    acceptChild(node->value, true); 
}

void ASTPrinter::visit(VarDeclarationNode* node) {
    visitNode("[VarDecl] name: '" + node->name + "'");
    acceptChild(node->typeDefinition, true);
}

void ASTPrinter::visit(TypeDeclarationNode* node) {
    visitNode("[TypeDecl] name: '" + node->name + "'");
    acceptChild(node->typeDefinition, true);
}

void ASTPrinter::visit(SimpleTypeNode* node) {
    visitNode("[SimpleType] '" + node->name + "'");
}

void ASTPrinter::visit(RangeTypeNode* node) {
    visitNode("[SubrangeType]");
    acceptChild(node->lowBound, false);
    acceptChild(node->highBound, true);
}

void ASTPrinter::visit(EnumeratedTypeNode* node) {
    std::string elementsStr = "";
    for (const auto& el : node->elements) elementsStr += el + ", ";
    if (!elementsStr.empty()) { elementsStr.pop_back(); elementsStr.pop_back(); } // Hapus ", " terakhir
    
    visitNode("[EnumeratedType] elements: [" + elementsStr + "]");
}

void ASTPrinter::visit(ArrayTypeNode* node) {
    visitNode("[ArrayType]");
    acceptChild(node->indexType, false);
    acceptChild(node->elementType, true);
}

void ASTPrinter::visit(RecordTypeNode* node) {
    visitNode("[RecordType]");
    for (size_t i = 0; i < node->fields.size(); ++i) {
        bool last = (i + 1 == node->fields.size());
        acceptChild(node->fields[i], last);
    }
}

void ASTPrinter::visit(ProcedureDeclarationNode* node) {
    visitNode("[ProcedureDecl] name: '" + node->name + "'");
    
    size_t totalChildren = node->parameters.size() + (node->body ? 1 : 0);
    size_t currentIdx = 0;

    for (const auto& param : node->parameters) {
        currentIdx++;
        acceptChild(param, currentIdx == totalChildren);
    }
    if (node->body) {
        currentIdx++;
        acceptChild(node->body, currentIdx == totalChildren);
    }
}

void ASTPrinter::visit(FunctionDeclarationNode* node) {
    visitNode("[FunctionDecl] name: '" + node->name + "'");
    
    size_t totalChildren = 1 + node->parameters.size() + (node->body ? 1 : 0); 
    size_t currentIdx = 0;

    currentIdx++;
    acceptChild(node->returnType, currentIdx == totalChildren);

    for (const auto& param : node->parameters) {
        currentIdx++;
        acceptChild(param, currentIdx == totalChildren);
    }
    if (node->body) {
        currentIdx++;
        acceptChild(node->body, currentIdx == totalChildren);
    }
}

void ASTPrinter::visit(IntegerLiteralNode* node) {
    visitNode("[IntLiteral] value: " + std::to_string(node->value));
}

void ASTPrinter::visit(RealLiteralNode* node) {
    visitNode("[RealLiteral] value: " + std::to_string(node->value));
}

void ASTPrinter::visit(StringLiteralNode* node) {
    visitNode("[StringLiteral] value: '" + node->value + "'");
}

void ASTPrinter::visit(BooleanLiteralNode* node) {
    visitNode("[BooleanLiteral] value: " + std::string(node->value ? "True" : "False"));
}

void ASTPrinter::visit(CharLiteralNode* node) {
    visitNode("[CharLiteral] value: '" + std::string(1, node->value) + "'");
}

void ASTPrinter::visit(VarAccessNode* node) {
    visitNode("[VarAccess] name: '" + node->name + "'");
}

void ASTPrinter::visit(UnaryOpNode* node) {
    visitNode("[UnaryOp] op: '" + node->op + "'");
    acceptChild(node->operand, true);
}

void ASTPrinter::visit(BinaryOpNode* node) {
    visitNode("[BinaryOp] " + node->op);
    if (node->left) {
        acceptChild(node->left, node->right == nullptr);
    }
    if (node->right) {
        acceptChild(node->right, true); 
    }
}

void ASTPrinter::visit(FunctionCallNode* node) {
    visitNode("[FunctionCall] " + node->name);
    for (size_t i = 0; i < node->args.size(); ++i) {
        bool isLastArgument = (i == node->args.size() - 1);
        acceptChild(node->args[i], isLastArgument);
    }
}

void ASTPrinter::visit(FieldAccessNode* node) {
    visitNode("[FieldAccess] ." + node->fieldName);
    if (node->target) {
        acceptChild(node->target, true);
    }
}

void ASTPrinter::visit(ArrayAccessNode* node) {
    visitNode("[ArrayAccess]");
    if (node->target) {
        acceptChild(node->target, node->index == nullptr); 
    }
    
    if (node->index) {
        acceptChild(node->index, true);
    }
}

void ASTPrinter::visit(CompoundStatementNode* node) {
    visitNode("[CompoundStatement]");
    for (size_t i = 0; i < node->statements.size(); ++i) {
        bool isLast = (i == node->statements.size() - 1);
        acceptChild(node->statements[i], isLast);
    }
}

void ASTPrinter::visit(AssignmentStatementNode* node) {
    visitNode("[AssignmentStatement] :=");
    std::vector<std::shared_ptr<ASTNode>> children;
    if (node->target) children.push_back(node->target);
    if (node->value) children.push_back(node->value);
    
    for (size_t i = 0; i < children.size(); ++i) {
        acceptChild(children[i], i == children.size() - 1);
    }
}

void ASTPrinter::visit(IfStatementNode* node) {
    visitNode("[IfStatement]");
    std::vector<std::shared_ptr<ASTNode>> children;
    if (node->condition) children.push_back(node->condition);
    if (node->thenBranch) children.push_back(node->thenBranch);
    if (node->elseBranch) children.push_back(node->elseBranch);
    
    for (size_t i = 0; i < children.size(); ++i) {
        acceptChild(children[i], i == children.size() - 1);
    }
}

void ASTPrinter::visit(WhileLoopNode* node) {
    visitNode("[WhileLoop]");
    std::vector<std::shared_ptr<ASTNode>> children;
    if (node->condition) children.push_back(node->condition);
    if (node->body) children.push_back(node->body);
    
    for (size_t i = 0; i < children.size(); ++i) {
        acceptChild(children[i], i == children.size() - 1);
    }
}

void ASTPrinter::visit(ForLoopNode* node) {
    std::string direction = node->isDownTo ? "downto" : "to";
    visitNode("[ForLoop] " + node->counterVar + " " + direction);
    std::vector<std::shared_ptr<ASTNode>> children;
    if (node->startValue) children.push_back(node->startValue);
    if (node->endValue) children.push_back(node->endValue);
    if (node->body) children.push_back(node->body);
    
    for (size_t i = 0; i < children.size(); ++i) {
        acceptChild(children[i], i == children.size() - 1);
    }
}

void ASTPrinter::visit(RepeatUntilNode* node) {
    visitNode("[RepeatUntilLoop]");
    std::vector<std::shared_ptr<ASTNode>> children;
    for (const auto& stmt : node->body) {
        if (stmt) children.push_back(stmt);
    }
    // Terakhir masukkan kondisi until
    if (node->condition) {
        children.push_back(node->condition);
    }
    
    for (size_t i = 0; i < children.size(); ++i) {
        acceptChild(children[i], i == children.size() - 1);
    }
}

void ASTPrinter::visit(ProcedureCallNode* node) {
    visitNode("[ProcedureCall] " + node->name);
    for (size_t i = 0; i < node->args.size(); ++i) {
        bool isLast = (i == node->args.size() - 1);
        acceptChild(node->args[i], isLast);
    }
}

void ASTPrinter::visit(CaseStatementNode* node) {
    visitNode("[CaseStatement]");
    std::vector<std::shared_ptr<ASTNode>> children;
    if (node->expression) children.push_back(node->expression);
    for (const auto& caseBlock : node->cases) {
        for (const auto& constant : caseBlock.first) {
            children.push_back(constant);
        }
        if (caseBlock.second) {
            children.push_back(caseBlock.second);
        }
    }

    // if (node->elseBranch) children.push_back(node->elseBranch);
    
    for (size_t i = 0; i < children.size(); ++i) {
        acceptChild(children[i], i == children.size() - 1);
    }
}

void ASTPrinter::visit(EmptyStatementNode* node) {}