#include "SymbolTable.hpp"
#include <iostream>
#include <string>

void SymbolTable::initPredefined()
{
    currentBlock = 0; 
    tabTop = 0;
    // index 0 : nullpointer untuk 'NULL';
    tab[0] = {"NULL", 0, ObjectType::TYPE, DataType::UNKNOWN, 0, 0, 0, 0};
    //       identifier, link, obj,           type,    ref, nrm,  level, address.
    tab[1] = {"INTEGER", 0, ObjectType::TYPE, DataType::INTEGER, 0, 1, 0, 4}; // size 4 byte
    tab[2] = {"REAL", 1, ObjectType::TYPE, DataType::REAL, 0, 1, 0, 8};       // size 8 byte
    tab[3] = {"CHAR", 2, ObjectType::TYPE, DataType::CHAR, 0, 1, 0, 1};       // size 1 byte
    tab[4] = {"BOOLEAN", 3, ObjectType::TYPE, DataType::BOOLEAN, 0, 1, 0, 1}; // size 1 byte
    tab[5] = {"STRING", 4, ObjectType::TYPE, DataType::STRING, 0, 1, 0, 255}; // size 255 byte

    std::vector<std::string> reserveWord = {
        "AND", "NOT", "MOD", "DIV", "IF", "BEGIN", "CASE", "CONST",
        "DOWNTO", "DO", "ELSE", "END", "FOR", "OF", "OR", "PROGRAM",
        "REPEAT", "THEN", "TO", "TYPE", "VAR", "UNTIL", "WHILE",
        "ARRAY", "RECORD", "FUNCTION", "PROCEDURE"};

    int currentLink = 5;
    int i = 6;
    for (const std::string &word : reserveWord) {
        if (i >= (int)tab.size()) tab.resize(tab.size() + 20);
        tab[i].name = word;
        tab[i].obj = ObjectType::RESERVE;
        tab[i].type = DataType::VOID;
        tab[i].link = currentLink;
        currentLink = i;
        i++;
    }

    // Set btab[0].last to point to the last reserved word FIRST
    tabTop = i - 1;
    btab[0].last = tabTop;
    
    // Now add predefined I/O procedures at level 0 (global)
    // enter() will use btab[0].last + 1 to find the next index
    enter("writeln", ObjectType::PROCEDURE, DataType::VOID, 0);
    enter("write", ObjectType::PROCEDURE, DataType::VOID, 0);
    enter("read", ObjectType::PROCEDURE, DataType::VOID, 0);
    enter("readln", ObjectType::PROCEDURE, DataType::VOID, 0);

    enter("True", ObjectType::CONSTANT, DataType::BOOLEAN, 0);
    enter("False", ObjectType::CONSTANT, DataType::BOOLEAN, 0);
};

int SymbolTable::lookup(std::string name, int lastIndex) const
{
    int i = lastIndex;
    while (i > 0) {
        if (tab[i].name == name) return i;
        i = tab[i].link;
    }
    return 0;
}

int SymbolTable::enter(std::string name, ObjectType kind, DataType type, int lev)
{
    // Cari indeks kosong berikutnya.
    tabTop++;
    int index = tabTop;

    // Jika kapasitas tabel hampir penuh, perbesar ukurannya
    if (index >= (int)tab.size()) {
        tab.resize(tab.size() + 20);
    }

    // Masukkan data ke dalam tabel
    tab[index].name = name;
    tab[index].obj = kind;
    tab[index].type = type;
    tab[index].lev = lev;

    tab[index].link = btab[currentBlock].last;
    btab[currentBlock].last = index;

    return index;
}

SymbolTable::~SymbolTable() = default;