# ⚙️ Arion Compiler: Milestone 1 - Lexical Analysis

[cite_start]A **custom lexical analyzer (lexer)** written in **C/C++ GNU**[cite: 117]. [cite_start]It takes raw source code in `.txt` format for the Arion programming language and converts it into a sequence of meaningful tokens using a **Deterministic Finite Automata (DFA)**.

[cite_start]This project is part of the IF2224 - Formal Language and Automata Theory course at STEI ITB.

-----

\<div align="center"\>

### 👥 Contributors

\<div align="center" id="contributor"\>
\<strong\>
\<table align="center"\>
\<tr align="center"\>
\<td\>NIM\</td\>
\<td\>Name\</td\>
\<td\>GitHub\</td\>
\</tr\>
\<tr align="center"\>
\<td\>13524024\</td\>
\<td\>Billie Bhaskara Wibawa\</td\>
\<td\>\<a href="[https://github.com/](https://github.com/billie-bytes)"\>@billie-bytes\</a\>\</td\>
\</tr\>
\<tr align="center"\>
\<td\>13524050\</td\>
\<td\>Raysha Erviandika Putra\</td\>
\<td\>\<a href="[https://github.com/](https://github.com/Arbane557)"\>@Arbane557\</a\>\</td\>
\</tr\>
\<tr align="center"\>
\<td\>13524058\</td\>
\<td\>Muhammad Naufal Romi Annafi\</td\>
\<td\>\<a href="[https://github.com/](https://github.com/)"\>@naufalromi\</a\>\</td\>
\</tr\>
\<tr align="center"\>
\<td\>13524084\</td\>
\<td\>Dzaki Ahmad Al Hussainy\</td\>
\<td\>\<a href="[https://github.com/](https://github.com/HussainDzaki)"\>@HussainDzaki\</a\>\</td\>
\</tr\>
\</table\>
\</strong\>
\</div\>

## ✨ Features

### This project contains:

1.  [cite_start]**Tokenizes Arion Source Code**: Reads `.txt` files containing Arion code and outputs a documented list of tokens.
2.  [cite_start]**DFA-based Engine**: Processes characters one-by-one entirely based on manual DFA rules.
3.  [cite_start]**No External Libraries**: Built from scratch without the use of any lexer generation tools or libraries.
4.  [cite_start]**Makefile Integration**: Streamlined compilation using Makefile.
5.  [cite_start]**Modular Design**: Structured to easily integrate into the future pipeline (Syntax Analysis, Semantic Analysis, Intermediate Code Generation).

## 📁 Repository Structure

  * [cite_start]`src/`: Directory to store all C/C++ source code.
  * [cite_start]`doc/`: Directory to store the project report.
  * [cite_start]`test/`: Directory to store all input/output `.txt` files used for testing.


## 🔧 Installation

### 📦 Requirements

  - [cite_start]**C/C++ GNU Compiler** (GCC/G++) [cite: 117]
  - [cite_start]**Make** [cite: 119]

### ⬇️ Installing Dependencies

  - 🖼 **Windows**

    1.  Download and install MinGW-w64 from an official distributor.
    2.  Ensure `gcc`, `g++`, and `make` (or `mingw32-make`) are added to your system PATH.
    3.  Verify: `gcc --version` and `make --version`

  - 🐧 **Linux / UNIX**

    1.  Open your terminal.
    2.  Install the build essentials:

    <!-- end list -->

    ```bash
       sudo apt update
       sudo apt install build-essential
    ```

    3.  Verify:

    <!-- end list -->

    ```bash
       gcc --version && make --version
    ```

-----

## 🛠️ How To Run

1.  Clone the Repository

    ```bash
    git clone https://github.com/naufalromi/JEE-Tubes-IF2224-2026.git
    ```

2.  Navigate to the Source Directory and Compile

    ```bash
    cd src
    make
    ```

3.  Run the Lexer Program

    ```bash
    ./lexer <path to .txt input>
    ```