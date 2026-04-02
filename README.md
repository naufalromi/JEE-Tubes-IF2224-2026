# ⚙️ Arion Compiler: Milestone 1 - Lexical Analysis

A **custom lexical analyzer (lexer)** written in **GNU C/C++ (GCC/G++)**. It takes raw source code in `.txt` format for the Arion programming language and converts it into a sequence of meaningful tokens using a **Deterministic Finite Automata (DFA)**.

This project is part of the IF2224 - Formal Language and Automata Theory course at STEI ITB.

---

<div align="center">

### 👥 Contributors

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

## ✨ Features

### This project contains:

1. **Tokenizes Arion Source Code**: Reads `.txt` files containing Arion code and outputs a documented list of tokens.  
2. **DFA-based Engine**: Processes characters one-by-one entirely based on manual DFA rules.  
3. **No External Libraries**: Built from scratch without the use of any lexer generation tools or libraries.  
4. **Makefile Integration**: Streamlined compilation using Makefile.  
5. **Modular Design**: Structured to easily integrate into the future pipeline (Syntax Analysis, Semantic Analysis, Intermediate Code Generation).  

---

## 📁 Repository Structure

- `src/`: Directory to store all C/C++ source code  
- `doc/`: Directory to store the project report  
- `test/`: Directory to store all input/output `.txt` files used for testing  

---

## 🔧 Installation

### 📦 Requirements

- **GNU C/C++ Compiler (GCC/G++)**  
- **Make**  

---

### ⬇️ Installing Dependencies

#### 🖼 Windows
1. Download and install MinGW-w64 from an official distributor  
2. Ensure `gcc`, `g++`, and `make` (or `mingw32-make`) are added to your system PATH  
3. Verify:
```bash
   g++ --version
   make --version
````

#### 🐧 Linux / UNIX

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

## 🛠️ How To Run

1. Clone the repository

```bash
git clone https://github.com/naufalromi/JEE-Tubes-IF2224-2026.git
```

2. Navigate to the source directory and compile

```bash
cd JEE-Tubes-IF2224-2026
make all
```

3. Run the lexer program

```bash
make run
```

<div align="center">

### 📊 Work Distribution

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
Implementation: Alpha Scanner, DFA State Builder<br>
Diagram: Alpha Scanner, Report<br>
Guidance: DFA State Implementation
</td>
<td>25%</td>
</tr>

<tr align="center">
<td>Raysha Erviandika Putra</td>
<td>13524050</td>
<td>
Implementation: Error Tracing, Numeric Scanner, Text Scanner, Symbol Scanner<br>
Diagram: Alpha Scanner
</td>
<td>25%</td>
</tr>

<tr align="center">
<td>Muhammad Naufal Romi Annafi</td>
<td>13524058</td>
<td>
Diagram: Global DFA, Numeric Scanner, Text Scanner, Symbol Scanner<br>
Report
</td>
<td>25%</td>
</tr>

<tr align="center">
<td>Dzaki Ahmad Al Hussainy</td>
<td>13524084</td>
<td>
Pre-Planning: Lexer Architecture Design<br>
Testing & Implementation<br>
Initial Program Structure
</td>
<td>25%</td>
</tr>

</table>
</strong>
</div>