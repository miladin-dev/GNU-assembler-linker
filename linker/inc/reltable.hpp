#ifndef RELTABLE_H_
#define RELTABLE_H_

#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <iomanip>

using namespace std;

typedef struct {
    int offset;
    string type;
    string value;
} reldata;

class RelTable {
public:
    RelTable(string name);

    void push_entry(int offset, string type, string value);
    void print_reltable(stringstream &stream);
    string get_name();
    void set_displ(int d);

    int reltable_size();

    reldata& at(int i);

private:
    vector<reldata> relt_entry;
    string name;
    int current_displ;
};

#endif