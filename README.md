# Arion Compiler: Milestone 4 & 5 - Intermediate Code Generator & Interpreter

This project implements a complete custom compiler and virtual machine developed in GNU C/C++ for the Arion programming language. The pipeline includes a lexical analyzer, a Recursive Descent syntax parser, a semantic analyzer, an Intermediate Code (IC) generator, and a custom Stack-based Virtual Machine (Interpreter). 

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

### This project contains a full Front-End to Back-End pipeline:

1. **Tokenization (Lexer)**: Converts source text into token streams using a manual DFA-based scanner.
2. **Recursive Descent Parsing**: Builds Parse Tree structures from Arion grammar rules.
3. **Semantic Validation**: Performs scope resolution, type compatibility checks, and constructs TAB, ATAB, and BTAB symbol tables.
4. **Intermediate Code Generation**: Traverses the Decorated AST to synthesize flat, Three-Address Code (TAC) instructions (e.g., `LOD`, `STO`, `JMP`, `JPC`, `OPR`).
5. **Stack-Based Virtual Machine (Interpreter)**: A custom execution engine featuring a Program Counter, Stack Pointer, and Base Pointer to manage memory, variable scopes (Static/Dynamic links), and subroutine calls.
6. **File-Based Execution**: Includes an `ICParser` capable of loading and executing standalone `.ic` bytecode files.
7. **Runtime Error Handling**: Active VM protection against stack overflow, stack underflow, invalid jump targets, and division by zero.
8. **Interactive I/O**: Full support for `readln`, `read`, `writeln`, and `write` standard procedures.
9. **No External Generators**: Implemented entirely from scratch in C++ without tools like Lex or Yacc.

---

## Repository Structure
```text
.
├── README.md
├── src
│   ├── codegen
│   │   ├── CodeGenerator.cpp
│   │   └── CodeGenerator.hpp
│   ├── common
│   │   ├── DataType.hpp
│   │   ├── Error.cpp
│   │   ├── Error.hpp
│   │   ├── Instruction.cpp
│   │   ├── Instruction.hpp
│   │   ├── NodeType.hpp
│   │   ├── ObjectType.hpp
│   │   ├── Reader.cpp
│   │   ├── Reader.hpp
│   │   ├── RuntimeValue.cpp
│   │   ├── RuntimeValue.hpp
│   │   ├── Token.cpp
│   │   ├── Token.hpp
│   │   ├── Tree.cpp
│   │   ├── Tree.hpp
│   │   ├── Writer.cpp
│   │   └── Writer.hpp
│   ├── interpreter
│   │   ├── ICParser.cpp
│   │   ├── ICParser.hpp
│   │   ├── Interpreter.cpp
│   │   └── Interpreter.hpp
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
- `4` Intermediate Code Generator
- `5` Interpreter (Run Program)

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
