#include "SymbolTable.hpp"
#include <string>

void SymbolTable::initPredefined()
{
    // index 0 : nullpointer untuk 'NULL';
    tab[0] = {"", 0, ObjectType::TYPE, DataType::UNKNOWN, 0, 0, 0, 0};
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
        if (i >= 33) break;
        tab[i].name = word;
        tab[i].obj = ObjectType::RESERVE;
        tab[i].type = DataType::VOID;
        tab[i].link = currentLink;
        currentLink = i;
        i++;
    }

    btab[0].last = currentLink;
};

int SymbolTable::lookup(std::string name, int lastIndex)
{
    int i = lastIndex;
    while (i > 0) {
        if (tab[i].name == name) return i;
        i = tab[i].link;
    }
    return 0;
}

int SymbolTable::enter(std::string name, ObjectType kind, DataType type, int lev) {
    // Cari indeks kosong berikutnya. 
    // Kita cek berdasarkan data terakhir yang masuk di blok (scope) saat ini.
    int index = btab.back().last + 1; 

    // Jika kapasitas tabel hampir penuh, perbesar ukurannya
    if (index >= (int)tab.size()) {
        tab.resize(tab.size() + 20); 
    }

    // Masukkan data ke dalam tabel
    tab[index].name = name;
    tab[index].obj = kind;
    tab[index].type = type;
    tab[index].lev = lev;
    
    // Hubungkan dengan identifier sebelumnya (Linked-List gaya tabel)
    tab[index].link = btab.back().last; 

    // Update penunjuk 'terakhir' di blok (scope) saat ini
    btab.back().last = index;

    return index;
}

SymbolTable::~SymbolTable() = default;