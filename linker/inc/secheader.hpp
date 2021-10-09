#ifndef _SECDATA_H
#define _SECDATA_H

#include <string>
#include <ostream>
#include <sstream>
#include <list>
#include <vector>
#include <fstream>


using namespace std;

typedef struct
{
    int idx;
    string name;
    int size;
    int offset;
} shdrdata;


class SectionHeader
{
public:
    void add(string name, int size, int offset);
    void print_shdr(stringstream &stream);

    bool exists(string name);
    void set_size(string name, int size);
    void set_offset(string name, int off);
    void update_offset(vector<pair<string,string>> &place);

    int get_displacement(string name);
    int get_size(string name);
    int get_offset(string name);

    void pop_displacement(string name);

    shdrdata& at(int i);

private:
    vector<shdrdata> shdr_entry;
    vector<pair<string,int>> displacements;
    vector<pair<string,int>> cursor;
    int id = 0;
};

#endif