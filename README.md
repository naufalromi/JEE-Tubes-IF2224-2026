# Arion Compiler: Milestone 3 - Semantic Analyzer

This project implements a custom lexical analyzer, syntax analyzer, and semantic analyzer developed in GNU C/C++ for the Arion programming language. The lexer processes source code with a Deterministic Finite Automaton (DFA), then the Recursive Descent parser builds a Parse Tree from grammar rules. The semantic phase transforms the Parse Tree into a decorated AST and validates program meaning through symbol resolution, type checking, scope handling, and table construction.

This project is part of the IF2224 - Formal Language and Automata Theory course at STEI ITB.

---

<div align="center">

### Contributors

</div>

<div align="center" id="contributor">
<strong>
<table align="center">
<tr align="center">
<td>NIM</td>
<td>Name</td>
<td>GitHub</td>
</tr>

<tr align="center">
<td>13524024</td>
<td>Billie Bhaskara Wibawa</td>
<td><a href="https://github.com/billie-bytes">@billie-bytes</a></td>
</tr>

<tr align="center">
<td>13524050</td>
<td>Raysha Erviandika Putra</td>
<td><a href="https://github.com/Arbane557">@Arbane557</a></td>
</tr>

<tr align="center">
<td>13524058</td>
<td>Muhammad Naufal Romi Annafi</td>
<td><a href="https://github.com/naufalromi">@naufalromi</a></td>
</tr>

<tr align="center">
<td>13524084</td>
<td>Dzaki Ahmad Al Hussainy</td>
<td><a href="https://github.com/HussainDzaki">@HussainDzaki</a></td>
</tr>

</table>
</strong>
</div>

---

## Features

### This project contains:

1. **Tokenizes Arion Source Code**: Converts source text into token streams using a manual DFA-based scanner.
2. **Recursive Descent Parsing**: Builds Parse Tree structures from Arion grammar rules.
3. **AST Builder**: Converts Parse Tree into AST nodes used by semantic processing.
4. **Semantic Validation**: Performs scope resolution, identifier checks, type compatibility checks, function/procedure checks, and assignment validation.
5. **Symbol Table Suite**: Produces and manages TAB, ATAB, and BTAB structures.
6. **Decorated AST Output**: Annotates AST nodes with semantic information (types, references, scope levels).
7. **Error Handling**: Reports lexical, syntax, and semantic errors with source locations.
8. **Makefile Integration**: Build and run flow through Make targets.
9. **No External Parser Generators**: Implemented from scratch in C++ without lexer/parser generator tools.

---

## Repository Structure
```
.
├── README.md
├── src
│   ├── common
│   │   ├── DataType.hpp
│   │   ├── Error.cpp
│   │   ├── Error.hpp
│   │   ├── NodeType.hpp
│   │   ├── ObjectType.hpp
│   │   ├── Reader.cpp
│   │   ├── Reader.hpp
│   │   ├── Token.cpp
│   │   ├── Token.hpp
│   │   ├── Tree.cpp
│   │   ├── Tree.hpp
│   │   ├── Writer.cpp
│   │   └── Writer.hpp
│   ├── lexer
│   │   ├── Lexer.cpp
│   │   ├── Lexer.hpp
│   │   ├── LexerState.hpp
│   │   ├── SpecificScanners.cpp
│   │   └── SpecificScanners.hpp
│   ├── semantic
│   │   ├── ASTBuilder.cpp
│   │   ├── ASTBuilder.hpp
│   │   ├── ASTNode.hpp
│   │   ├── ASTPrinter.cpp
│   │   ├── ASTPrinter.hpp
│   │   ├── ASTVisitor.hpp
│   │   ├── SemanticAnalyzer.cpp
│   │   ├── SemanticAnalyzer.hpp
│   │   ├── SemanticVisitor.cpp
│   │   ├── SemanticVisitor.hpp
│   │   ├── SymbolTable.cpp
│   │   └── SymbolTable.hpp
│   ├── syntax
│   │   ├── Parser.cpp
│   │   └── Parser.hpp
│   └── main.cpp
└── test
   ├── lexer
   ├── parser
   └── semantic
```
---

## Installation

### Requirements

- **GNU C/C++ Compiler (GCC/G++)**  
- **Make**  

---

### Installing Dependencies

#### Windows
1. Download and install MinGW-w64 from an official distributor  
2. Ensure `gcc`, `g++`, and `make` (or `mingw32-make`) are added to your system PATH  
3. Verify:
```bash
   g++ --version
   make --version
````

#### Linux / UNIX

1. Open your terminal
2. Install build essentials:

```bash
sudo apt update
sudo apt install build-essential
```
3. Verify:

```bash
g++ --version && make --version
```

---

## How To Run

1. Clone the repository

```bash
git clone https://github.com/naufalromi/JEE-Tubes-IF2224-2026.git
```

2. Navigate to the repository and build

```bash
cd JEE-Tubes-IF2224-2026
make clean
make all
```

3. Run the compiler

```bash
make run
```

4. Choose analysis mode in the interactive menu:
- `1` Lexical Analyzer
- `2` Syntax Analyzer
- `3` Semantic Analyzer

5. Choose testcase number based on the selected mode folder.

Semantic mode will:
- Build and analyze the decorated AST
- Print semantic errors when present
- Allow semantic display options (Decorated AST, TAB, ATAB, BTAB, or all)

<div align="center">

### Work Distribution

</div>

<div align="center">
<strong>
<table align="center">
<tr align="center">
<td>Name</td>
<td>NIM</td>
<td>Workload</td>
<td>Percentage</td>
</tr>

<tr align="center">
<td>Billie Bhaskara Wibawa</td>
<td>13524024</td>
<td>
Report and Coding
</td>
<td>25%</td>
</tr>

<tr align="center">
<td>Raysha Erviandika Putra</td>
<td>13524050</td>
<td>
Report and Coding
</td>
<td>25%</td>
</tr>

<tr align="center">
<td>Muhammad Naufal Romi Annafi</td>
<td>13524058</td>
<td>
Report and Coding
</td>
<td>25%</td>
</tr>

<tr align="center">
<td>Dzaki Ahmad Al Hussainy</td>
<td>13524084</td>
<td>
Report and Coding
</td>
<td>25%</td>
</tr>

</table>
</strong>
</div>
