#pragma once
#include "../common/DataType.hpp"
#include "../common/ObjectType.hpp"
#include <vector>

struct TabEntry {
    std::string name;   // identifiers
    int link;           // indeks ke ident sebelumnya dalam scope sama
    ObjectType obj;     // jenis objek
    DataType type;      // tipe dasar
    int ref;            // indeks ke atab atau btab
    int nrm;            // 1 = normal, 0 = by-reference
    int lev;            // lexical level (0 = global, 1 = dalam prosedur, 2 = dalam prosedur di dalam prosedur, dst)
    long adr;           // nilai/alamat/offset
};

struct AtabEntry {
    DataType xtyp;      // tipe indeks
    DataType etyp;      // tipe elemen
    int eref;           // ref detail jika elemen komposit
    int low;            // batas bawah
    int high;           // batas atas
    int elsz;           // ukuran elemen
    int size;           // total ukuran array
};


struct BtabEntry {
    int last;           // indeks identifier terakhir di blok ini
    int lpar;           // indeks parameter terakhir
    int psze;           // total ukuran parameter
    int vsze;           // total ukuran variabel lokal
};

class SymbolTable
{
public:
    std::vector<TabEntry> tab;
    std::vector<AtabEntry> atab;
    std::vector<BtabEntry> btab;

    int tabTop = 0;
    int currentBlock = 0;

    int enter(std::string name, ObjectType kind, DataType type, int lev);
    int lookup(std::string name, int lastIndex);
    
    SymbolTable() {
        tab.resize(33);
        atab.resize(1);
        btab.resize(1);
        initPredefined();
    }
    
    ~SymbolTable();

private:
    void initPredefined();
};
