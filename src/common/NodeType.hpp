#pragma once

// redefine because there is variables too
// tabbed -> done
enum class NodeType {
        Program,
        ProgramHeader,          
        DeclarationPart,            // done <DeclarationPart> -> <ConstDeclaration> <TypeDeclaration> <VarDeclaration> <SubprogramDeclaration>
    ConstDeclaration,               // done <ConstDeclaration> -> const <Constant> {; <Constant>}
    Constant,                       // nope <Constant> -> ident = (number | char | string)
    TypeDeclaration,            // nope <TypeDeclaration> -> type ident = <Type>
    VarDeclaration,             // nope <VarDeclaration> -> var <IdentifierList> : <Type> {; <IdentifierList> : <Type>}                 
    IdentifierList,             // nope <IdentifierList> -> ident {, ident}
    Type,                       // nope <Type> -> <ArrayType> | <Range> | <Enumerated> | <RecordType> | ident
    ArrayType,                  // nope <ArrayType> -> array [ <Range> ] of <Type>
    Range,                      // nope <Range> -> number .. number
    Enumerated,                 // nope <Enumerated> -> ( ident {, ident} )
    RecordType,                 // nope <RecordType> -> record <FieldList> end                       
    FieldList,                  // nope <FieldList> -> <FieldPart> {; <FieldPart>}
    FieldPart,                  // nope <FieldPart> -> <IdentifierList> : <Type>
    SubprogramDeclaration,      // nope <SubprogramDeclaration> -> <ProcedureDeclaration> | <FunctionDeclaration>    
    ProcedureDeclaration,       // nope <ProcedureDeclaration> -> procedure ident <FormalParameterList> ; <Block>
    FunctionDeclaration,        // nope <FunctionDeclaration> -> function ident <FormalParameterList> : <Type> ; <Block>    
    Block,                      // nope <Block> -> begin <StatementList> end
    FormalParameterList,        // nope <FormalParameterList> -> ( <ParameterGroup> {; <ParameterGroup>} )           
    ParameterGroup,             // nope <ParameterGroup> -> [ var ] <IdentifierList> : <Type>
        CompoundStatement,          // done <CompoundStatement> -> begin <StatementList> end
    StatementList,              // nope <StatementList> -> <Statement> {; <Statement>}
    Statement,                  // nope <Statement> -> <AssignmentStatement> | <IfStatement> | <CaseStatement> | <WhileStatement> | <RepeatStatement> | <ForStatement> | <ProcedureCall>
    AssignmentStatement,        // nope <AssignmentStatement> ->s ident := <Expression>
    IfStatement,                // nope <IfStatement> -> if <Expression> then <Statement> [ else <Statement> ]
    CaseStatement,              // nope <CaseStatement> -> case <Expression> of <CaseBlock> end
    CaseBlock,                  // nope <CaseBlock> -> <Constant> : <Statement> {; <Constant> : <Statement>}
    WhileStatement,             // nope <WhileStatement> -> while <Expression> do <Statement> 
    RepeatStatement,            // nope <RepeatStatement> -> repeat <StatementList> until <Expression>
    ForStatement,               // nope <ForStatement> -> for ident := <Expression> (to | downto) <Expression> do <Statement>
    ProcedureCall,              // nope <ProcedureCall> -> ident ( [ <Expression> {, <Expression>} ] )
    ParameterList,              // nope <ParameterList> -> <Expression> {, <Expression>}
    Expression,                 // nope <Expression> -> <SimpleExpression> [ <RelationalOperator> <SimpleExpression> ]
    SimpleExpression,           // nope <SimpleExpression> -> [ <AdditiveOperator> ] <Term> { <AdditiveOperator> <Term> }
    Term,                       // nope <Term> -> <Factor> { <MultiplicativeOperator> <Factor> }
    Factor,                     // nope <Factor> -> ident | number | char | string | ( <Expression> ) | not <Factor>
    RelationalOperator,         // nope <RelationalOperator> -> = | <> | < | <= | > | >=      
    AdditiveOperator,           // nope <AdditiveOperator> -> + | - | or
    MultiplicativeOperator,     // nope <MultiplicativeOperator> -> * | / | div | mod | and
    
    Variable,
    ComponentVariable,
    IndexList,
    Error,

    // Terminals
    Ident,
    ConstSy,
    IntCon,
    RealCon,
    CharCon,
    String,
    ProgramSy,
    Semicolon,
    VarSy,
    Comma,
    Colon,
    ArraySy,
    LBrack,
    RBrack,
    OfSy,
    RecordSy,
    EndSy,
    ProcedureSy,
    FunctionSy,
    LParent,
    RParent,
    BeginSy,
    Becomes,
    Plus,
    Minus,
    Times,
    RDiv,
    IDiv,
    IMod,
    AndSy,
    OrSy,
    NotSy,
    IfSy,
    ThenSy,
    ElseSy,
    CaseSy,
    OfSy2,
    WhileSy,
    DoSy,
    RepeatSy,
    UntilSy,
    ForSy,
    ToSy,
    DownToSy,
    Period,
    Eql,
    Neq,
    Gtr,
    Geq,
    Lss,
    Leq,
    TypeSy,
    Unknown,
    Literal,
    BinaryOperation,
    ParameterRefGroup
};

// Utility to convert enum to string for printing
inline const char* nodeTypeToString(NodeType type) {
    switch (type) {
        case NodeType::Program: return "<program>";
        case NodeType::ProgramHeader: return "<program-header>";
        case NodeType::DeclarationPart: return "<declaration-part>";
        case NodeType::ConstDeclaration: return "<const-declaration>";
        case NodeType::Constant: return "<constant>";
        case NodeType::TypeDeclaration: return "<type-declaration>";
        case NodeType::VarDeclaration: return "<var-declaration>";
        case NodeType::IdentifierList: return "<identifier-list>";
        case NodeType::Type: return "<type>";
        case NodeType::ArrayType: return "<array-type>";
        case NodeType::Range: return "<range>";
        case NodeType::Enumerated: return "<enumerated>";
        case NodeType::RecordType: return "<record-type>";
        case NodeType::FieldList: return "<field-list>";
        case NodeType::FieldPart: return "<field-part>";
        case NodeType::SubprogramDeclaration: return "<subprogram-declaration>";
        case NodeType::ProcedureDeclaration: return "<procedure-declaration>";
        case NodeType::FunctionDeclaration: return "<function-declaration>";
        case NodeType::Block: return "block";
        case NodeType::FormalParameterList: return "<formal-parameter-list>";
        case NodeType::ParameterGroup: return "<parameter-group>";
        case NodeType::CompoundStatement: return "<compound-statement>";
        case NodeType::StatementList: return "<statement-list>";
        case NodeType::Statement: return "<statement>";
        case NodeType::AssignmentStatement: return "<assignment-statement>";
        case NodeType::IfStatement: return "<if-statement>";
        case NodeType::CaseStatement: return "<case-statement>";
        case NodeType::CaseBlock: return "<case-block>";
        case NodeType::WhileStatement: return "<while-statement>";
        case NodeType::RepeatStatement: return "<repeat-statement>";
        case NodeType::ForStatement: return "<for-statement>";
        case NodeType::ProcedureCall: return "<procedure-call>";
        case NodeType::ParameterList: return "<parameter-list>";
        case NodeType::Expression: return "<expression>";
        case NodeType::SimpleExpression: return "<simple-expression>";
        case NodeType::Term: return "<term>";
        case NodeType::Factor: return "<factor>";
        case NodeType::RelationalOperator: return "<relational-operator>";
        case NodeType::AdditiveOperator: return "<additive-operator>";
        case NodeType::MultiplicativeOperator: return "<multiplicative-operator>";
        case NodeType::Variable: return "<variable>";
        case NodeType::ComponentVariable: return "<component-variable>";
        case NodeType::IndexList: return "<index-list>";
        // Terminals
        case NodeType::Ident: return "ident";
        case NodeType::ConstSy: return "constsy";
        case NodeType::IntCon: return "intcon";
        case NodeType::RealCon: return "realcon";
        case NodeType::CharCon: return "charcon";
        case NodeType::String: return "string";
        case NodeType::ProgramSy: return "programsy";
        case NodeType::Semicolon: return "semicolon";
        case NodeType::VarSy: return "varsy";
        case NodeType::Comma: return "comma";
        case NodeType::Colon: return "colon";
        case NodeType::ArraySy: return "arraysy";
        case NodeType::LBrack: return "lbrack";
        case NodeType::RBrack: return "rbrack";
        case NodeType::OfSy: return "ofsy";
        case NodeType::RecordSy: return "recordsy";
        case NodeType::EndSy: return "endsy";
        case NodeType::ProcedureSy: return "proceduresy";
        case NodeType::FunctionSy: return "functionsy";
        case NodeType::LParent: return "lparent";
        case NodeType::RParent: return "rparent";
        case NodeType::BeginSy: return "beginsy";
        case NodeType::Becomes: return "becomes";
        case NodeType::Plus: return "plus";
        case NodeType::Minus: return "minus";
        case NodeType::Times: return "times";
        case NodeType::RDiv: return "rdiv";
        case NodeType::IDiv: return "idiv";
        case NodeType::IMod: return "imod";
        case NodeType::AndSy: return "andsy";
        case NodeType::OrSy: return "orsy";
        case NodeType::NotSy: return "notsy";
        case NodeType::IfSy: return "ifsy";
        case NodeType::ThenSy: return "thensy";
        case NodeType::ElseSy: return "elsy";
        case NodeType::CaseSy: return "casesy";
        case NodeType::OfSy2: return "ofsy";
        case NodeType::WhileSy: return "whilesy";
        case NodeType::DoSy: return "dosy";
        case NodeType::RepeatSy: return "repeatsy";
        case NodeType::UntilSy: return "untilsy";
        case NodeType::ForSy: return "forsy";
        case NodeType::ToSy: return "tosy";
        case NodeType::DownToSy: return "downtosy";
        case NodeType::Period: return "period";
        case NodeType::Eql: return "eql";
        case NodeType::Neq: return "neq";
        case NodeType::Gtr: return "gtr";
        case NodeType::Geq: return "geq";
        case NodeType::Lss: return "lss";
        case NodeType::Leq: return "leq";
        case NodeType::TypeSy: return "typesy";
        default: return "unknown";
    }
}
