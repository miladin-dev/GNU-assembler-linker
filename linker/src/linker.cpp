#include <stdio.h>
#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include "../inc/secheader.hpp"
#include "../inc/linker.hpp"




Linker::Linker(string output_name, list<string> &input_name, vector<pair<string,string>> &place_command, bool is_hex)
{
    shdr_table = new SectionHeader();
    sym_table = new SymbolTable();

    input_files = new ifstream[input_name.size()];
    curr_table = -1;
    to_place = place_command;

    if(!create_tables(output_name, input_name))
        return;


    output.open(output_name);

    if(is_hex)
    {
        if(!edit_offsets())
            return;

        relocate();
    }
    stringstream ss;
    stringstream file_stream;
    shdr_table->print_shdr(ss);
    sym_table->st_print(ss);

    print_content(file_stream);
    
    for(SectionData* data : sec_data_list)
    {
        data->print_sectiondata(ss);
    }

    for(RelTable* rt : rel_table_list)
    {
        rt->print_reltable(ss);
    }
    
    if(is_hex)
        output << file_stream.str();
    else 
        output << ss.str();
    //cout << ss.str();
}

Linker::~Linker()
{
    delete shdr_table;
    delete sym_table;

    for(SectionData* data : sec_data_list)
    {
        delete data;
    }

    for(RelTable* rt : rel_table_list)
    {
        delete rt;
    }
}

bool Linker::edit_offsets()
{
    for(auto & s1 : to_place)
    {
        int start1 = hexToInt(s1.second);
        int end1 = start1 + shdr_table->get_size(s1.first);
        
        for(auto &s2 : to_place)
        {
            if(s1.first != s2.first)
            {
                int start2 = hexToInt(s2.second);
                if(start1 < start2 && end1 > start2)
                {
                    cerr << "Section overloading. Exiting..." << endl;
                    return false;
                }
            }
        }
    }

    shdr_table->update_offset(to_place);


    for(int i = 0; i < sym_table->symtable_size(); i++)
    {
        symdata & el = sym_table->at(i);

        if(el.mysec == "*UND*" && el.name != "UND"){
            cout << "Undefined symbol " << el.name << ". Exiting..." << endl;
            return false;
        }

        if(el.is_section)
            el.value = shdr_table->get_offset(el.name);
        
        if(sym_table->is_global(el.name))
        {
            el.value += shdr_table->get_offset(el.mysec);
        }
    }

    for(SectionData* sd : sec_data_list)
    {
        string name = sd->get_name();
        int off = shdr_table->get_offset(name);

        sd->set_sec_offset(off);

        sorted_sections.push_back(make_pair(off, sd));
    }

    std::sort(sorted_sections.begin(), sorted_sections.end());
    
    return true;
}

void Linker::relocate()
{
    for(SectionData* sd : sec_data_list)
    {
        vector<int>& sec_vector = sd->get_vector();

        for(RelTable* rt : rel_table_list)
        {
            if(rt->get_name() == sd->get_name())
            {
                for(int i = 0; i < rt->reltable_size(); i++)
                {
                    reldata & rd = rt->at(i);
                    if(rd.type == "R_DATA_VN_16")
                    {
                        stringstream ss;
                        ss << hex << sec_vector[rd.offset + 1];
                        ss << hex << sec_vector[rd.offset];
                        string f = ss.str();
                        int built = hexToInt(f);
                        built += sym_table->get_symValue(rd.value);
                        sec_vector[rd.offset] = built & 0xFF;
                        sec_vector[rd.offset + 1] = (built >> 8) & 0xFF;
                    }
                    else if(rd.type == "R_VN_16")
                    {
                        stringstream ss;
                        ss << hex << sec_vector[rd.offset];
                        ss << hex << sec_vector[rd.offset + 1];
                        string f = ss.str();
                        int built = hexToInt(f);
                        built += sym_table->get_symValue(rd.value);
                        sec_vector[rd.offset] = (built >> 8) & 0xFF;
                        sec_vector[rd.offset + 1] = built & 0xFF;
                    }
                    else if(rd.type == "R_VN_PC_16")
                    {
                        stringstream ss;
                        ss << hex << sec_vector[rd.offset];
                        ss << hex << sec_vector[rd.offset + 1];
                        string f = ss.str();
                        int built = hexToInt(f);
                        built += sym_table->get_symValue(rd.value) - (sym_table->get_symValue(rt->get_name()) + rd.offset);
                        sec_vector[rd.offset] = (built >> 8) & 0xFF;
                        sec_vector[rd.offset + 1] = built & 0xFF;
                    }
                }
            }
        }
    }
}

bool Linker::create_tables(string &output, list<string> &input_name)
{
    int size = input_name.size();

    for(int i = 0; i < size; i++)
    {
        string file = input_name.front();
        input_name.pop_front();

        input_files[i].open(file);

        string line;
        smatch matched;

        for(line; getline(input_files[i], line);)
        {
            if(is_blank(line))
                continue;

            if(is_info(line))
            {
                getline(input_files[i], line);
                continue;
            }

            if(curr_table == SECTION_HEADERS)
            {
                parse_shdr_entry(line);
            }
            if(curr_table == SYMBOL_TABLE)
            {
                if(!parse_symt_entry(line)){
                    return false;
                }
            }
            if(curr_table == SECTION_DATA)
            {
                parse_section_data(line);
            }
            if(curr_table == REL_TABLE)
            {
                parse_rel_table(line);
            }


        }

        input_files[i].close();
    }

    return true;
}

void Linker::parse_section_data(string &line)
{
    regex data("^\\d{4}:\\s(.*)$");
    smatch bytes;
    if(regex_search(line, bytes, data))
    {
        string to_split = bytes[1];

        separate_bytes(to_split);
    }

}

void Linker::parse_rel_table(string &line)
{
    regex rel_entry("^" RXP_HEX "\\s+" "(\\w+)" "\\s+" RXP_ALF "\\s*$");
    smatch match;
    regex_search(line, match, rel_entry);

    int offset = hexToInt(match[1]);
    string addr_type = match[2];
    string value = match[3];

    
    rel_table->push_entry(offset, addr_type, value);



}

void Linker::separate_bytes(string to_split)
{
    stringstream ss(to_split);
    string byte;
    
    while(getline(ss, byte, ' '))
    {
        sec_data->add_byte(hexToInt(byte));
    }

}

void Linker::parse_shdr_entry(string &line)
{
    regex entry("^\\s*\\d+\\s*(\\w+)\\s*(\\w+)\\s*(\\d+)\\s*$");
    smatch match;

    if(regex_search(line, match, entry))
    {
        string name = match[1];
        int size = hexToInt(match[2]);
        int offset = hexToInt(match[3]);

        if(shdr_table->exists(name))
        {
            if(!to_connect_exists(name)){
                sections_to_connect.push_back(name);
            }

            shdr_table->set_size(name, size);
        }
        else
        {
            shdr_table->add(name, size, offset);
        } 

    }
}

bool Linker::parse_symt_entry(string &line)
{
    regex symt_entry("^" RXP_DIGIT "\\s+" RXP_ALF "\\s+" "(\\w+)" "\\s+" RXP_ALF "\\s+" RXP_ALF "\\s*$");
    regex extern_entry("^" RXP_DIGIT "\\s+" RXP_ALF "\\s+\\*UND\\*\\s*$");
    smatch match;

    if(regex_search(line, match, symt_entry))
    {
        string name = match[2];
        string bind = match[4];
        string my_sec = match[5];
        int value = hexToInt(match[3]);
        bool is_equ = false;

        if(my_sec == "*ABS*")
            is_equ = true;
        
        if(sym_table->exists(name))
        {
            if(shdr_table->exists(name))    //If current symbol exists as section
            {
                if(my_sec == name)
                {
                    return true;
                }
                else {
                    cerr << "Symbol '" << name << "' already exists. Exiting..." << endl;
                    return false;
                }
            }
            else if(bind == "LOCAL")
            {
                if(sym_table->is_local(name) 
                                    && my_sec != name                    //my_Sec != name da li uvozim simbol, a ne sekciju
                                    && my_sec != sym_table->get_mysection(name)) //Da li uvozim lokalan simbol, a sekcije su im iste
                {
                    sym_table->add(name, value, bind, my_sec, is_equ);
                    return true;
                }
                else 
                {
                    cerr << "Symbol '" << name << "' already exists. Exiting..." << endl;
                    return false;
                }
            }
            else if(bind == "GLOBAL")
            {
                if(sym_table->is_extern(name))
                {
                    sym_table->redefine_symbol(name, value, bind, my_sec);
                    return true;
                }
                else 
                {
                    cerr << "Symbol '" << name << "' already exists. Exiting..." << endl;
                    return false;
                }
            }
        }
        else
        {
            sym_table->add(name, value, bind, my_sec, is_equ);
            return true;
        }
    }
    else if(regex_search(line, match, extern_entry))
    {
        string name = match[2];
        string my_sec = "*UND*";

        if(name == "UND")
            return true; //vec je dodat pri kreaciji

        if(sym_table->exists(name))
        {
            if(sym_table->is_local(name))
            {
                cerr << "Symbol '" << name << "' already exists. Exiting..." << endl;
                return false;
            }
            else
                return true; // ne radi nista ako je vec globalan ili eksteran
        }

        sym_table->add(name, UND, "", my_sec, false);
        return true;
    }

    return false;
}

bool Linker::is_info(string &line)
{
    regex sec_inf("^Section header:.*$");
    regex symt_info("^Symbol table.*$");
    regex secdata_info(".*Content of section\\s+#?(\\w+).*");
    regex reltable_info(".*Relocation table of section\\s+(\\w+).*");
    smatch match;

    if(regex_search(line, sec_inf)){
        curr_table = SECTION_HEADERS;
        return true;
    }
    else if(regex_search(line, symt_info))
    {
        curr_table = SYMBOL_TABLE;
        return true;
    }
    else if(regex_search(line, match, secdata_info))
    {
        string sec_name = match[1];
        if(to_connect_exists(sec_name))
        {
            for(SectionData* sec : sec_data_list)
            {
                if(sec->get_name() == sec_name){
                    sec_data = sec;
                }
            }
        }
        else 
        {
            sec_data = new SectionData(match[1]);
            sec_data_list.push_back(sec_data);
        }


        curr_table = SECTION_DATA;

        return true;
    }
    else if(regex_search(line, match, reltable_info))
    {
        string sec_name = match[1];

        if(to_connect_exists(sec_name))
        {
            for(RelTable* rt : rel_table_list)
            {
                if(rt->get_name() == sec_name){
                    rel_table = rt;
                    shdr_table->pop_displacement(match[1]);
                    int displ = shdr_table->get_displacement(match[1]);
                    
                    rel_table->set_displ(displ);
                }
            }
        }
        else 
        {
            rel_table = new RelTable(match[1]);
            rel_table_list.push_back(rel_table);
        }



        curr_table = REL_TABLE;
        return true;
    }
    
    return false;
    
}

bool Linker::is_blank(string &line)
{
    regex blank("^\\s*$");

    if(regex_search(line, blank))
        return true;
    else return false;
}

void Linker::print_content(stringstream& stream)
{
    stream << right << setfill(' ') << setw(7);
    for(int i = 0; i < 8; i++)
        stream << "0" << hex << i << " ";

    bool print = true;
    for(int i = 0; i < sorted_sections.size(); i++)
    {
        auto &p = sorted_sections.at(i);
        SectionData* sd = p.second;
        vector<int>& sd_vec = sd->get_vector();
        int counter = 0;
        int addr = sd->get_sec_offset();
        
        for(int el : sd_vec)
        {
            if(counter % 8 == 0 && print){
                stream << endl;
                stream << right << setfill('0') << setw(4) << hex << (addr - addr%8) << ": ";
                addr += 8;
            }

            //If start address isnt aligned
            int r = sd->get_sec_offset() % 8;
            if(counter == 0)
            {
                if(print == true)
                {
                    for(int i = 0; i < r; i++)
                    {
                        stream << "00 ";
                        counter++;
                    }
                }
                else {
                    counter += r;
                    addr += 8;
                }

                print = true;
            }
            counter++;
            stream << right <<  setfill('0') << setw(2) <<  hex << el << " ";
        }
        
        if((sd->get_sec_offset() + sd->data_size()) % 8 != 0)
        {
            //If it's not last section
            if(i != sorted_sections.size() - 1)
            {
                auto& next = sorted_sections.at(i+1);
                if(next.second->get_sec_offset() == sd->get_sec_offset() + sd->data_size())
                {
                    print = false;
                }
                else
                {
                    int to_fill = 8 - (sd->get_sec_offset() + sd->data_size()) % 8;
                    for(int j = 0; j < to_fill; j++)
                        stream << "00 ";
                    print = true;
                }
            }
            else {
                //If its last section fill with zeros till end of line
                int to_fill = 8 - (sd->get_sec_offset() + sd->data_size()) % 8;
                    for(int j = 0; j < to_fill; j++)
                        stream << "00 ";
            }
        }
        else print = true;
    }
}

bool Linker::to_connect_exists(string name)
{
    for(auto & el : sections_to_connect)
    {
        if(el == name)
            return true;
    }

    return false;
}

int Linker::hexToInt(string to_convert)
{
    const char* cstr = to_convert.c_str();
    int symbol2 = (int)strtol(cstr, NULL, 16);
 
    return symbol2;
}