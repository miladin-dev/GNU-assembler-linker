#ifndef SECDATA_H
#define SECDATA_H

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>


using namespace std;

class SectionData
{
public:
    SectionData(string name);
    void add_byte(int byte);
    void print_sectiondata(stringstream &stream);
    string get_name();

    int data_size();
    vector<int> & get_vector();

    void set_sec_offset(int off);
    int get_sec_offset();

private:
    int section_offset;
    string name;
    vector<int> data;
};


#endif