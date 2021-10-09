#ifndef LINKER_H_
#define LINKER_H_
#include <string>
#include <list>
#include <vector>
#include <fstream>
#include "../inc/secheader.hpp"
#include "../inc/symtable.hpp"
#include "../inc/secdata.hpp"
#include "../inc/reltable.hpp"

#define RXP_DIGIT "([0-9]+)"
#define RXP_ALF "([\\*A-Za-z_]+)"
#define RXP_HEX "([ABCDEFabcdef0-9]+)"
#define SECTION_HEADERS 0
#define SYMBOL_TABLE 1
#define SECTION_DATA 2
#define REL_TABLE 3
#define UND -1

using namespace std;

class Linker
{
public:
    Linker(string output_name, list<string> &input_name, vector<pair<string,string>> &place_command, bool is_hex);
    ~Linker();
    
    bool is_info(string &line);
    bool is_blank(string &line);

    void parse_shdr_entry(string &line);
    bool parse_symt_entry(string &line);
    void parse_section_data(string &line);
    void parse_rel_table(string &line);
    bool create_tables(string &output, list<string> &input_names);
    bool edit_offsets();
    void relocate();

    void separate_bytes(string to_split);
    void print_content(stringstream &stream);
    bool to_connect_exists(string name);
    static int hexToInt(string to_convert);

private:
    SectionHeader* shdr_table;
    SectionData* sec_data;
    SectionData* hex_data;
    RelTable* rel_table;
    list<SectionData*> sec_data_list;
    list<RelTable*> rel_table_list;
    vector<string> sections_to_connect;
    vector<pair<string, string>> to_place;
    vector<pair<int, SectionData*>> sorted_sections;

    SymbolTable* sym_table;
    ifstream* input_files;
    ofstream output;
    int curr_table;
};

#endif