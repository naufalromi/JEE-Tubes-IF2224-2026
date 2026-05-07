# Arion Compiler: Milestone 2 - Syntax Analyzer

This project implements a custom lexical analyzer and syntax analyzer—developed in GNU C/C++ for the Arion programming language. The lexer reads raw .txt source code in Arion Programming Language and utilizes a Deterministic Finite Automaton (DFA) to convert it into a sequence of meaningful tokens. These tokens are then evaluated by a Recursive Descent parser based on strict non-terminal grammar rules to construct a hierarchical Parse Tree. Additionally, the system incorporates a panic-mode synchronization mechanism for robust syntax error detection and recovery.

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

1. **Tokenizes Arion Source Code**: Reads `.txt` files containing Arion code and outputs a documented list of tokens.  
2. **DFA-based Engine**: Processes characters one-by-one entirely based on manual DFA rules.
3. **Recursive Descent Parser**: Evaluates the tokenized data strictly against 42 non-terminal grammar rules.
4. **Parse Tree Generator**: Constructs a hierarchical, node-based representation of the validated source code.
5. **Error Handling**: Implements handler for error in lexical analyzer and syntax analyzer
4. **No External Libraries**: Built from scratch without the use of any lexer generation tools or libraries.  
5. **Makefile Integration**: Streamlined compilation using Makefile.  
6. **Modular Design**: Structured to easily integrate into the future pipeline (Semantic Analysis and Intermediate Code Generation).  

---

## Repository Structure
```
.
├── README.md
├── build
├── data
├── doc
│   └── Laporan-2-JEE.pdf
├── enumref.txt
├── jeecompiler
├── makefile
├── output
│   └── output.txt
├── src
│   ├── common
│   │   ├── Error.cpp
│   │   ├── Error.hpp
│   │   ├── NodeType.hpp
│   │   ├── Reader.cpp
│   │   ├── Reader.hpp
│   │   ├── Token.cpp
│   │   ├── Token.hpp
│   │   ├── Tree.cpp
│   │   ├── Tree.hpp
│   │   ├── Writer.cpp
│   │   └── Writer.hpp
│   ├── lexer
│   │   ├── Lexer.cpp
│   │   ├── Lexer.hpp
│   │   ├── LexerState.hpp
│   │   ├── SpecificScanners.cpp
│   │   └── SpecificScanners.hpp
│   ├── main.cpp
│   └── syntax
│       ├── Parser.cpp
│       └── Parser.hpp
└── test
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

2. Navigate to the source directory and compile

```bash
cd JEE-Tubes-IF2224-2026
make clean
make all
```

3. Run the lexer program

```bash
make run
```

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
