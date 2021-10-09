#ifndef _SECDATA_H
#define _SECDATA_H

#include <string>
#include <ostream>
#include <sstream>
#include <list>
#include <vector>
#include <fstream>
#include "symtable.hpp"

using namespace std;

typedef struct
{
    int idx;
    string name;
    int size;
    int offset;
} shdrdata;


class SectionData
{
public:
    vector<int> data;
    SectionData(string n);
    void add_word(int word);
    void add_word_instr(int word);
    void add_byte(int byte);

    void print_sectionData(stringstream &stream);

    string name; //radi ispisa u assembler konstrkotur
private:
};



class SectionHeader
{
public:
    SectionHeader();
    void add(string name, int size, int offset);
    void print_shdr(stringstream &stream);
    void set_size(string name, int size);

private:
    list<shdrdata> shdr_entry;
    SymbolTable* symt;
    int id = 0;
};

#endif