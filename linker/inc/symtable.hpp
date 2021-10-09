#ifndef _SYMTABLE_H
#define _SYMTABLE_H

#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <vector>

using namespace std;


typedef struct {
    int id;
    string name;
    int value;
    string bind;      // 0 - local , global - 1
    string mysec;
    
    bool is_section;
    bool is_equ;
} symdata;



class SymbolTable
{
public:
    SymbolTable();
    
    void add(string name, int value, string bind, string ndx, bool is_equ);
    static int get_id();
    void st_print(stringstream &stream);
    string print_mysection(symdata& el);

    bool is_global(string name);
    bool is_extern(string name);
    bool is_local(string name);
    bool is_equ(string name);

    int get_symValue(string name);
    int get_symId(string name);
    string get_mysection(string name);         //Vraca mysec - tj sekciju u kojoj je simbol definisan.

    symdata &at(int i);
    int symtable_size();

    void redefine_symbol(string name, int value, string bind, string mysec);
    void set_size(string name, int size);
    bool exists(string name);
    void set_global(string name);

private:
    static int st_ID;
    vector<symdata> symt_entry;
};

#endif