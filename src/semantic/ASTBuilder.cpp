#include "ASTBuilder.hpp"

std::shared_ptr<ProgramNode> ASTBuilder::buildProgram(std::shared_ptr<TreeNode> node) {
    if (!node) return nullptr;

    std::string programName = "Unknown";
    std::vector<std::shared_ptr<DeclarationNode>> declarations;
    std::shared_ptr<CompoundStatementNode> statements;

    for (const auto& child : node->children) {
        if (child->type == NodeType::ProgramHeader) {
            for (const auto& headerChild : child->children) {
                if (headerChild->type == NodeType::Ident) {
                    programName = headerChild->value;
                    break;
                }
            }
        }
        else if (child->type == NodeType::DeclarationPart) {
            declarations = buildDeclarationPart(child);
        }
        else if (child->type == NodeType::CompoundStatement) {
            statements = buildCompoundStatement(child);
        }
    }

    auto program = setSourceLocation(std::make_shared<ProgramNode>(programName), node);
    program->declarations = declarations;
    program->statements = statements;

    return program;
}

std::shared_ptr<BlockNode> ASTBuilder::buildBlock(std::shared_ptr<TreeNode> node) {
    auto blockNode = setSourceLocation(std::make_shared<BlockNode>(), node);
    if (!node) return blockNode;
    for (const auto& child : node->children) {
        if (child->type == NodeType::DeclarationPart) {
            blockNode->declarations = buildDeclarationPart(child);
        }
        else if (child->type == NodeType::CompoundStatement) {
            blockNode->statements = buildCompoundStatement(child);
        }
    }

    return blockNode;
}

std::vector<std::shared_ptr<DeclarationNode>> ASTBuilder::buildDeclarationPart(std::shared_ptr<TreeNode> node) {
    std::vector<std::shared_ptr<DeclarationNode>> allDecls;
    if (!node) return allDecls;

    for (const auto& child : node->children) {
        if (child->type == NodeType::ConstDeclaration) {
            auto consts = buildConstDeclarations(child);
            allDecls.insert(allDecls.end(), consts.begin(), consts.end());
        } 
        else if (child->type == NodeType::VarDeclaration) {
            auto vars = buildVarDeclarations(child);
            allDecls.insert(allDecls.end(), vars.begin(), vars.end());
        }
        else if (child->type == NodeType::TypeDeclaration) {
            auto types = buildTypeDeclarations(child);
            allDecls.insert(allDecls.end(), types.begin(), types.end());
        }
        else if (child->type == NodeType::ProcedureDeclaration) {
            allDecls.push_back(buildProcedureDeclaration(child));
        }
        else if (child->type == NodeType::FunctionDeclaration) {
            allDecls.push_back(buildFunctionDeclaration(child));
        }
    }
    return allDecls;
}

std::vector<std::shared_ptr<ConstDeclarationNode>> ASTBuilder::buildConstDeclarations(std::shared_ptr<TreeNode> node) {
    std::vector<std::shared_ptr<ConstDeclarationNode>> result;
    if (!node) return result;

    std::string currentName = "";

    for (const auto& child : node->children) {
        if (child->type == NodeType::Ident) {
            currentName = child->value;
        } 
        
        else if (child->type == NodeType::Constant) {
            auto literalValue = buildLiteral(child); 

            if (!currentName.empty() && literalValue != nullptr) {
                result.push_back(setSourceLocation(std::make_shared<ConstDeclarationNode>(currentName, literalValue), child));
                currentName = ""; 
            }
        }
    }

    return result;
}

std::vector<std::shared_ptr<TypeDeclarationNode>> ASTBuilder::buildTypeDeclarations(std::shared_ptr<TreeNode> node) {
    std::vector<std::shared_ptr<TypeDeclarationNode>> result;
    if (!node) return result;

    std::string currentName = "";

    for (const auto& child : node->children) {
        if (child->type == NodeType::Ident) {
            currentName = child->value;
        } 
        else if (child->type == NodeType::Type) {
            auto typeDefinition = buildType(child);
            
            if (!currentName.empty() && typeDefinition != nullptr) {
                result.push_back(setSourceLocation(std::make_shared<TypeDeclarationNode>(currentName, typeDefinition), child));
                currentName = "";
            }
        }
    }
    return result;
}

std::vector<std::shared_ptr<VarDeclarationNode>> ASTBuilder::buildVarDeclarations(std::shared_ptr<TreeNode> node) {
    std::vector<std::shared_ptr<VarDeclarationNode>> result;
    std::vector<std::string> tempNames; 

    for (const auto& child : node->children) {
        if (child->type == NodeType::IdentifierList) {
            for (const auto& idNode : child->children) {
                if (idNode->type == NodeType::Ident) {
                    tempNames.push_back(idNode->value);
                }
            }
        } 
        
        else if (child->type == NodeType::Type) {
            auto typeNode = buildType(child);
            for (const auto& name : tempNames) {
                result.push_back(setSourceLocation(std::make_shared<VarDeclarationNode>(name, typeNode), child));
            }
            tempNames.clear();
        }
    }
    return result;
}

std::shared_ptr<ProcedureDeclarationNode> ASTBuilder::buildProcedureDeclaration(std::shared_ptr<TreeNode> node) {
    if (!node) return nullptr;

    std::string procName = "Unknown";
    std::vector<std::shared_ptr<VarDeclarationNode>> parameters;
    std::shared_ptr<BlockNode> bodyNode = nullptr;

    for (const auto& child : node->children) {
        
        if (child->type == NodeType::Ident) {
            procName = child->value;
        }
        
        else if (child->type == NodeType::FormalParameterList) {
            parameters = buildFormalParameters(child);
        }
        
        else if (child->type == NodeType::Block) {
            bodyNode = buildBlock(child);
        }
    }
    auto procNode = setSourceLocation(std::make_shared<ProcedureDeclarationNode>(procName), node);
    procNode->parameters = parameters;
    procNode->body = bodyNode;

    return procNode;
}

std::shared_ptr<FunctionDeclarationNode> ASTBuilder::buildFunctionDeclaration(std::shared_ptr<TreeNode> node) {
    if (!node) return nullptr;

    std::string funcName = "Unknown";
    std::string returnTypeName = "Void";
    bool foundName = false;         
    
    std::vector<std::shared_ptr<VarDeclarationNode>> parameters;
    std::shared_ptr<BlockNode> bodyNode = nullptr;

    for (const auto& child : node->children) {
        
        if (child->type == NodeType::Ident) {
            if (!foundName) {
                funcName = child->value;
                foundName = true;
            } else {
                returnTypeName = child->value;
            }
        }
        
        else if (child->type == NodeType::FormalParameterList) {
            parameters = buildFormalParameters(child);
        }
        
        else if (child->type == NodeType::Block) {
            bodyNode = buildBlock(child);
        }
    }

    auto returnTypeNode = setSourceLocation(std::make_shared<SimpleTypeNode>(returnTypeName), node);

    auto funcNode = setSourceLocation(std::make_shared<FunctionDeclarationNode>(funcName, returnTypeNode), node);
    funcNode->parameters = parameters;
    funcNode->body = bodyNode;

    return funcNode;
}

std::vector<std::shared_ptr<VarDeclarationNode>> ASTBuilder::buildFormalParameters(std::shared_ptr<TreeNode> node) {
    std::vector<std::shared_ptr<VarDeclarationNode>> allParameters;
    if (!node) return allParameters;

    for (const auto& child : node->children) {
        if (child->type == NodeType::ParameterGroup) {
            
            std::vector<std::string> paramNames;
            std::shared_ptr<TypeNode> paramType = nullptr;

            for (const auto& groupChild : child->children) {
                
                if (groupChild->type == NodeType::IdentifierList) {
                    for (const auto& idNode : groupChild->children) {
                        if (idNode->type == NodeType::Ident) {
                            paramNames.push_back(idNode->value);
                        }
                    }
                }
                
                else if (groupChild->type == NodeType::Ident) {
                    paramType = setSourceLocation(std::make_shared<SimpleTypeNode>(groupChild->value), groupChild);
                }
            }

            if (paramType) {
                for (const auto& name : paramNames) {
                    allParameters.push_back(setSourceLocation(std::make_shared<VarDeclarationNode>(name, paramType), child));
                }
            }
        }
    }

    return allParameters;
}

std::shared_ptr<TypeNode> ASTBuilder::buildType(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    auto typeChild = node->children[0];

    if (typeChild->type == NodeType::Ident) {
        return setSourceLocation(std::make_shared<SimpleTypeNode>(typeChild->value), typeChild);
    } 
    else if (typeChild->type == NodeType::RecordType) {
        return buildRecordType(typeChild);
    } 
    else if (typeChild->type == NodeType::ArrayType) {
        return buildArrayType(typeChild);
    }
    else if (typeChild->type == NodeType::Range) {
        return buildRangeType(typeChild);
    }
    else if (typeChild->type == NodeType::Enumerated) {
        return buildEnumeratedType(typeChild);
    }

    return nullptr;
}

std::shared_ptr<TypeNode> ASTBuilder::buildRecordType(std::shared_ptr<TreeNode> node) {
    auto recordNode = setSourceLocation(std::make_shared<RecordTypeNode>(), node);

    for (const auto& child : node->children) {
        if (child->type == NodeType::FieldList) {
            
            for (const auto& fieldPart : child->children) {
                if (fieldPart->type == NodeType::FieldPart) {
                    
                    std::vector<std::string> tempNames;
                    std::shared_ptr<TypeNode> fieldType = nullptr;

                    for (const auto& partChild : fieldPart->children) {
                        if (partChild->type == NodeType::IdentifierList) {
                            for (const auto& idNode : partChild->children) {
                                if (idNode->type == NodeType::Ident) tempNames.push_back(idNode->value);
                            }
                        } 
                        else if (partChild->type == NodeType::Type) {
                            fieldType = buildType(partChild);
                        }
                    }

                    if (fieldType) {
                        for (const auto& name : tempNames) {
                            recordNode->fields.push_back(setSourceLocation(std::make_shared<VarDeclarationNode>(name, fieldType), fieldPart));
                        }
                    }
                }
            }
        }
    }
    return recordNode;
}

std::shared_ptr<TypeNode> ASTBuilder::buildArrayType(std::shared_ptr<TreeNode> node) {
    if (!node) return nullptr;

    std::shared_ptr<TypeNode> indexType = nullptr;
    std::shared_ptr<TypeNode> elementType = nullptr;

    for (const auto& child : node->children) {
        if (child->type == NodeType::Range) {
            indexType = buildRangeType(child);
        } 
        else if (child->type == NodeType::Ident) {
            indexType = setSourceLocation(std::make_shared<SimpleTypeNode>(child->value), child); 
        }
        else if (child->type == NodeType::Type) {
            elementType = buildType(child); 
        }
    }

    return setSourceLocation(std::make_shared<ArrayTypeNode>(indexType, elementType), node);
}

std::shared_ptr<TypeNode> ASTBuilder::buildRangeType(std::shared_ptr<TreeNode> node) {
    std::vector<std::shared_ptr<ExpressionNode>> bounds;
    for (const auto& child : node->children) {
        if (child->type == NodeType::Constant) {
            bounds.push_back(buildLiteral(child));
        }
    }
    if (bounds.size() == 2) {
        return setSourceLocation(std::make_shared<RangeTypeNode>(bounds[0], bounds[1]), node);
    }
    return nullptr;
}

std::shared_ptr<TypeNode> ASTBuilder::buildEnumeratedType(std::shared_ptr<TreeNode> node) {
    auto enumNode = setSourceLocation(std::make_shared<EnumeratedTypeNode>(), node);

    for (const auto& child : node->children) {
        if (child->type == NodeType::Ident) {
            enumNode->elements.push_back(child->value);
        }
    }
    return enumNode;
}

std::shared_ptr<ExpressionNode> ASTBuilder::buildExpression(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    if (node->type == NodeType::Expression) {
        std::shared_ptr<ExpressionNode> left = nullptr;
        std::shared_ptr<ExpressionNode> right = nullptr;
        std::string op = "";

        for (const auto& child : node->children) {
            if (child->type == NodeType::SimpleExpression) {
                if (!left) {
                    left = buildExpression(child); 
                } else {
                    right = buildExpression(child);
                }
            } 
            else if (child->type == NodeType::RelationalOperator) {
                if (!child->children.empty()) op = child->children[0]->value;
            }
        }

        if (left && right && !op.empty()) {
            return setSourceLocation(std::make_shared<BinaryOpNode>(op, left, right), node);
        }
        return left; 
    }

    if (node->type == NodeType::SimpleExpression) {
        std::shared_ptr<ExpressionNode> expr = nullptr;
        bool isNegative = false;
        std::string currentOp = "";

        for (const auto& child : node->children) {
            if (child->type == NodeType::Plus || child->type == NodeType::Minus) {
                if (!expr) isNegative = (child->type == NodeType::Minus);
            }
            // Cek Term
            else if (child->type == NodeType::Term) {
                auto termNode = buildExpression(child);
                
                if (!expr) {
                    expr = termNode;
                    if (isNegative) expr = setSourceLocation(std::make_shared<UnaryOpNode>("-", expr), child);
                } else {
                    expr = setSourceLocation(std::make_shared<BinaryOpNode>(currentOp, expr, termNode), child);
                }
            }
            else if (child->type == NodeType::AdditiveOperator) {
                if (!child->children.empty()) currentOp = child->children[0]->value;
            }
        }
        return expr;
    }

    if (node->type == NodeType::Term) {
        std::shared_ptr<ExpressionNode> expr = nullptr;
        std::string currentOp = "";

        for (const auto& child : node->children) {
            if (child->type == NodeType::Factor) {
                auto factorNode = buildExpression(child);
                
                if (!expr) {
                    expr = factorNode;
                } else {
                    expr = setSourceLocation(std::make_shared<BinaryOpNode>(currentOp, expr, factorNode), child);
                }
            }
            else if (child->type == NodeType::MultiplicativeOperator) {
                if (!child->children.empty()) currentOp = child->children[0]->value;
            }
        }
        return expr;
    }

    if (node->type == NodeType::Factor) {
        std::string unaryOp = "";
        
        for (const auto& child : node->children) {
            if (child->type == NodeType::NotSy || child->type == NodeType::Minus) {
                unaryOp = child->value;
            }
            else if (child->type == NodeType::Expression) {
                auto expr = buildExpression(child);
                return unaryOp.empty() ? expr : setSourceLocation(std::make_shared<UnaryOpNode>(unaryOp, expr), node);
            }
            else if (child->type == NodeType::IntCon || child->type == NodeType::RealCon || 
                     child->type == NodeType::CharCon || child->type == NodeType::String) {
                auto expr = buildLiteral(child);
                return unaryOp.empty() ? expr : setSourceLocation(std::make_shared<UnaryOpNode>(unaryOp, expr), node);
            }
            else if (child->type == NodeType::Variable) {
                auto expr = buildVariableAccess(child);
                return unaryOp.empty() ? expr : setSourceLocation(std::make_shared<UnaryOpNode>(unaryOp, expr), node);
            }
            else if (child->type == NodeType::ProcedureCall) {
                auto expr = buildFunctionCall(child);
                return unaryOp.empty() ? expr : setSourceLocation(std::make_shared<UnaryOpNode>(unaryOp, expr), node);
            }
        }
    }

    return nullptr;
}

std::shared_ptr<ExpressionNode> ASTBuilder::buildVariableAccess(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    std::shared_ptr<ExpressionNode> currentAccess = nullptr;

    for (const auto& child : node->children) {
        
        if (child->type == NodeType::Ident) {
            currentAccess = setSourceLocation(std::make_shared<VarAccessNode>(child->value), child);
        } 
        
        else if (child->type == NodeType::ComponentVariable && currentAccess != nullptr) {
            
            bool isArrayAccess = false;
            bool isRecordAccess = false;
            std::shared_ptr<TreeNode> indexListNode = nullptr;
            std::string fieldName = "";

            for (const auto& compChild : child->children) {
                if (compChild->type == NodeType::LBrack) {
                    isArrayAccess = true;
                } 
                else if (compChild->type == NodeType::IndexList) {
                    indexListNode = compChild;
                } 
                else if (compChild->type == NodeType::Period) {
                    isRecordAccess = true;
                } 
                else if (compChild->type == NodeType::Ident && isRecordAccess) {
                    fieldName = compChild->value;
                }
            }

            if (isArrayAccess && indexListNode) {
                for (const auto& idxChild : indexListNode->children) {
                    std::shared_ptr<ExpressionNode> singleIndex = nullptr;
                    
                    if (idxChild->type == NodeType::Ident) {
                        singleIndex = setSourceLocation(std::make_shared<VarAccessNode>(idxChild->value), idxChild);
                    } 
                    else if (idxChild->type == NodeType::IntCon || idxChild->type == NodeType::CharCon) {
                        singleIndex = buildLiteral(idxChild);
                    }
                    if (singleIndex) {
                        currentAccess = setSourceLocation(std::make_shared<ArrayAccessNode>(currentAccess, singleIndex), child);
                    }
                }
            }
            
            else if (isRecordAccess && !fieldName.empty()) {
                currentAccess = setSourceLocation(std::make_shared<FieldAccessNode>(currentAccess, fieldName), child);
            }
        }
    }

    return currentAccess;
}

std::shared_ptr<ExpressionNode> ASTBuilder::buildFunctionCall(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    std::string funcName = "";
    std::vector<std::shared_ptr<ExpressionNode>> args;

    for (const auto& child : node->children) {
        
        if (child->type == NodeType::Ident) {
            funcName = child->value;
        }
        else if (child->type == NodeType::ParameterList || child->type == NodeType::ParameterList) {
            
            for (const auto& paramChild : child->children) {                
                if (paramChild->type == NodeType::Expression) {
                    auto argExpr = buildExpression(paramChild);
                    if (argExpr) {
                        args.push_back(argExpr);
                    }
                }
                else if (paramChild->type == NodeType::String) {
                    args.push_back(buildLiteral(paramChild));
                }
            }
        }
    }

    if (funcName.empty()) {
        return nullptr;
    }

    auto callNode = setSourceLocation(std::make_shared<FunctionCallNode>(funcName), node);
    callNode->args = args;
    return callNode;
}

std::shared_ptr<ExpressionNode> ASTBuilder::buildLiteral(std::shared_ptr<TreeNode> node) {
    if (!node) return nullptr;

    bool hasSign = false;
    std::string signOp = "";
    std::shared_ptr<TreeNode> targetNode = node; 

    if (node->type == NodeType::Constant) {
        targetNode = nullptr; 
        
        for (const auto& child : node->children) {
            if (child->type == NodeType::Plus || child->type == NodeType::Minus) {
                hasSign = true;
                signOp = (child->type == NodeType::Plus) ? "+" : "-";
            }
            else if (child->type == NodeType::IntCon || child->type == NodeType::RealCon ||
                     child->type == NodeType::String || child->type == NodeType::CharCon ||
                     child->type == NodeType::Ident) {
                targetNode = child;
            }
        }
    }

    if (!targetNode) return nullptr; 

    std::shared_ptr<ExpressionNode> literalNode = nullptr;

    if (targetNode->type == NodeType::IntCon) {
        literalNode = setSourceLocation(std::make_shared<IntegerLiteralNode>(std::stoi(targetNode->value)), targetNode);
    } 
    else if (targetNode->type == NodeType::RealCon) {
        literalNode = setSourceLocation(std::make_shared<RealLiteralNode>(std::stod(targetNode->value)), targetNode);
    }
    else if (targetNode->type == NodeType::String) {
        literalNode = setSourceLocation(std::make_shared<StringLiteralNode>(targetNode->value), targetNode);
    }
    else if (targetNode->type == NodeType::CharCon) {
        char charVal = targetNode->value.empty() ? '\0' : targetNode->value[0]; 
        literalNode = setSourceLocation(std::make_shared<CharLiteralNode>(charVal), targetNode);
    }
    else if (targetNode->type == NodeType::Ident) {
        literalNode = setSourceLocation(std::make_shared<VarAccessNode>(targetNode->value), targetNode);
    }

    if (hasSign && literalNode != nullptr) {
        return setSourceLocation(std::make_shared<UnaryOpNode>(signOp, literalNode), node);
    }

    return literalNode;
}

std::shared_ptr<StatementNode> ASTBuilder::buildStatement(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) {
        return nullptr;
    }

    auto targetNode = node->children[0];

    if (targetNode->type == NodeType::AssignmentStatement) {
        return buildAssignmentStatement(targetNode);
    } 
    else if (targetNode->type == NodeType::CompoundStatement) {
        return buildCompoundStatement(targetNode);
    }
    else if (targetNode->type == NodeType::IfStatement) {
        return buildIfStatement(targetNode);
    }
    else if (targetNode->type == NodeType::CaseStatement) {
        return buildCaseStatement(targetNode);
    }
    else if (targetNode->type == NodeType::WhileStatement) {
        return buildWhileLoop(targetNode);
    }
    else if (targetNode->type == NodeType::ForStatement) {
        return buildForLoop(targetNode);
    }
    else if (targetNode->type == NodeType::RepeatStatement) {
        return buildRepeatUntil(targetNode);
    }
    else if (targetNode->type == NodeType::ProcedureCall) {
        return buildProcedureCall(targetNode);
    }
    // else if (targetNode->type == NodeType::EmptyStatement) {
    //     return std::make_shared<EmptyStatementNode>();
    // }

    return nullptr;
}
std::shared_ptr<CompoundStatementNode> ASTBuilder::buildCompoundStatement(std::shared_ptr<TreeNode> node) {
    auto compoundNode = setSourceLocation(std::make_shared<CompoundStatementNode>(), node);
    if (!node) return compoundNode;
    for (const auto& child : node->children) {
        if (child->type == NodeType::StatementList) {
            for (const auto& stmtChild : child->children) {
                if (stmtChild->type == NodeType::Statement) {
                    auto stmt = buildStatement(stmtChild);
                    if (stmt) {
                        compoundNode->statements.push_back(stmt);
                    }
                }
            }
        }
    }

    return compoundNode;
}

std::shared_ptr<AssignmentStatementNode> ASTBuilder::buildAssignmentStatement(std::shared_ptr<TreeNode> node) {
    if (!node) return nullptr;

    std::shared_ptr<ExpressionNode> targetVar = nullptr;
    std::shared_ptr<ExpressionNode> valueExpr = nullptr;

    for (const auto& child : node->children) {
        if (child->type == NodeType::Variable) {
            targetVar = buildVariableAccess(child); 
        } 
        else if (child->type == NodeType::Expression) {
            valueExpr = buildExpression(child);    
        }
    }

    return setSourceLocation(std::make_shared<AssignmentStatementNode>(targetVar, valueExpr), node);
}

std::shared_ptr<IfStatementNode> ASTBuilder::buildIfStatement(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    std::shared_ptr<ExpressionNode> condition = nullptr;
    std::shared_ptr<StatementNode> thenBranch = nullptr;
    std::shared_ptr<StatementNode> elseBranch = nullptr;
    bool inElseBlock = false;
    for (const auto& child : node->children) {
        if (child->type == NodeType::Expression) {
            condition = buildExpression(child);
        }
        else if (child->type == NodeType::ElseSy) {
            inElseBlock = true;
        }
        else if (child->type == NodeType::Statement) {
            if (!inElseBlock) {
                thenBranch = buildStatement(child);
            } else {
                elseBranch = buildStatement(child);
            }
        }
    }
    return setSourceLocation(std::make_shared<IfStatementNode>(condition, thenBranch, elseBranch), node);
}

std::shared_ptr<CaseStatementNode> ASTBuilder::buildCaseStatement(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    std::shared_ptr<ExpressionNode> caseExpr = nullptr;
    
    std::vector<std::shared_ptr<TreeNode>> caseBlocksQueue;

    for (const auto& child : node->children) {
        if (child->type == NodeType::Expression) {
            caseExpr = buildExpression(child);
        }
        else if (child->type == NodeType::CaseBlock) {
            caseBlocksQueue.push_back(child);
        }
    }

    // Buat node AST-nya
    auto caseStmtNode = setSourceLocation(std::make_shared<CaseStatementNode>(caseExpr), node);
    size_t i = 0;
    while (i < caseBlocksQueue.size()) {
        auto currentBlock = caseBlocksQueue[i++];
        std::vector<std::shared_ptr<ExpressionNode>> currentConstants;
        std::shared_ptr<StatementNode> currentStmt = nullptr;
        for (const auto& child : currentBlock->children) {
            if (child->type == NodeType::Constant) {
                auto constNode = buildLiteral(child);
                if (constNode) currentConstants.push_back(constNode);
            }
            else if (child->type == NodeType::Statement) {
                currentStmt = buildStatement(child);
            }
            else if (child->type == NodeType::CaseBlock) {
                caseBlocksQueue.push_back(child); 
            }
        }
        if (!currentConstants.empty() && currentStmt != nullptr) {
            caseStmtNode->cases.push_back({currentConstants, currentStmt});
        }
    }
    return caseStmtNode;
}

std::shared_ptr<WhileLoopNode> ASTBuilder::buildWhileLoop(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    std::shared_ptr<ExpressionNode> condition = nullptr;
    std::shared_ptr<StatementNode> body = nullptr;

    for (const auto& child : node->children) {
        if (child->type == NodeType::Expression) {
            condition = buildExpression(child);
        }
        else if (child->type == NodeType::CompoundStatement) {
            body = buildCompoundStatement(child);
        }
        else if (child->type == NodeType::Statement) {
            body = buildStatement(child);
        }
    }

    return setSourceLocation(std::make_shared<WhileLoopNode>(condition, body), node);
}

std::shared_ptr<ForLoopNode> ASTBuilder::buildForLoop(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    std::string counterVar = "";
    std::shared_ptr<ExpressionNode> startValue = nullptr;
    std::shared_ptr<ExpressionNode> endValue = nullptr;
    bool isDownTo = false;
    std::shared_ptr<StatementNode> body = nullptr;
    for (const auto& child : node->children) {
        if (child->type == NodeType::Ident) {
            if (counterVar.empty()) {
                counterVar = child->value;
            }
        }
        else if (child->type == NodeType::Expression) {
            if (!startValue) {
                startValue = buildExpression(child);
            } else {
                endValue = buildExpression(child);
            }
        }
        else if (child->type == NodeType::DownToSy) {
            isDownTo = true;
        }
        else if (child->type == NodeType::ToSy) {
            isDownTo = false;
        }
        else if (child->type == NodeType::CompoundStatement) {
            body = buildCompoundStatement(child);
        }
        else if (child->type == NodeType::Statement) {
            body = buildStatement(child);
        }
    }
    return setSourceLocation(std::make_shared<ForLoopNode>(counterVar, startValue, endValue, isDownTo, body), node);
}

std::shared_ptr<RepeatUntilNode> ASTBuilder::buildRepeatUntil(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    std::shared_ptr<ExpressionNode> condition = nullptr;
    std::vector<std::shared_ptr<StatementNode>> bodyStatements;

    for (const auto& child : node->children) {
        if (child->type == NodeType::StatementList) {
            for (const auto& stmtChild : child->children) {
                if (stmtChild->type == NodeType::Statement) {
                    auto stmt = buildStatement(stmtChild);
                    if (stmt) {
                        bodyStatements.push_back(stmt);
                    }
                }
            }
        }
        else if (child->type == NodeType::Expression) {
            condition = buildExpression(child);
        }
    }

    auto repeatNode = setSourceLocation(std::make_shared<RepeatUntilNode>(condition), node);
    
    repeatNode->body = std::move(bodyStatements);

    return repeatNode;
}

std::shared_ptr<ProcedureCallNode> ASTBuilder::buildProcedureCall(std::shared_ptr<TreeNode> node) {
    if (!node || node->children.empty()) return nullptr;

    std::string procName = "";
    std::vector<std::shared_ptr<ExpressionNode>> args;

    for (const auto& child : node->children) {
        if (child->type == NodeType::Ident) {
            procName = child->value;
        }
        
        else if (child->type == NodeType::ParameterList || child->type == NodeType::ParameterList) {
            for (const auto& paramChild : child->children) {
                if (paramChild->type == NodeType::Expression) {
                    auto argExpr = buildExpression(paramChild);
                    if (argExpr) {
                        args.push_back(argExpr);
                    }
                }
                else if (paramChild->type == NodeType::String) {
                    args.push_back(buildLiteral(paramChild));
                }
            }
        }
    }

    if (procName.empty()) {
        return nullptr;
    }

    auto callNode = setSourceLocation(std::make_shared<ProcedureCallNode>(procName), node);
    callNode->args = args;
    return callNode;
}