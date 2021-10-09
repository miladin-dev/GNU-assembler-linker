#ifndef _SYMTABLE_H
#define _SYMTABLE_H

#include <iostream>
#include <string>
#include <sstream>
#include <list>

using namespace std;

#define UND -2
#define LOCAL 0
#define GLOBAL 1
#define IS_EQU 1
#define R_VN_PC16 0
#define R_VN_16 1


typedef struct {
    int id;
    string name;
    int value;
    short bind;      // 0 - local , global - 1
    int mysec;
    int size;
    
    bool is_section;
    bool is_equ;
} symdata;

typedef struct {
    int offset;
    string type;
    string value;
} reldata;

class SymbolTable
{
public:
    SymbolTable();
    
    void add(string name, int value, short bind, int ndx, bool is_equ);
    static int get_id();
    void st_print(stringstream &stream);
    string print_mysection(symdata& el);

    bool is_global(string name);
    bool is_equ(string name);

    int get_symValue(string symbol_name);
    int get_symId(string symbol_name);
    int get_sectionId(string symbol_name);         //Vraca mysec - tj sekciju u kojoj je simbol definisan.
    string get_symName(int id);

    void set_size(string name, int size);
    bool symbol_exists(string name);
    void set_global(string name);

private:
    static int st_ID;
    list<symdata> symt_entry;
};


// ----------------------------- REL TABLE -----------------------------
class RelTable {
public:
    RelTable(string name);

    void push_entry(int offset, string type, string value);
    void print_reltable(stringstream &stream);
    string get_name();

private:
    list<reldata> relt_entry;
    string name;
};

#endif