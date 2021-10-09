#include "../inc/symtable.hpp"
#include <iomanip>

int SymbolTable::st_ID = 0;


SymbolTable::SymbolTable()
{
    symdata sd;
    sd.id = st_ID++;
    sd.name = "UND";
    sd.value = -1;
    sd.bind = "";
    sd.mysec = "*UND*";
    sd.is_equ = false;

    symt_entry.push_back(sd);
}


void SymbolTable::add(string name, int value, string bind, string ndx, bool is_equ)
{
    symdata sd;
    sd.id = st_ID++;
    sd.name = name;
    sd.value = value;
    sd.bind = bind;
    sd.mysec = ndx;
    sd.is_equ = is_equ;

    if(name == ndx)
        sd.is_section = true;
    else sd.is_section = false;

    symt_entry.push_back(sd);
}

bool SymbolTable::exists(string name)
{
    for(int i = 0; i < symt_entry.size(); i++)
    {
        if(name == symt_entry.at(i).name)
            return true;
    }

    return false;
}

bool SymbolTable::is_global(string name)
{
    for(int i = 0; i < symt_entry.size(); i++)
    {
        if(name == symt_entry.at(i).name)
        {
          if(symt_entry.at(i).bind == "GLOBAL")
            return true;
          else return false;
        }   
    }

    return false;
}

bool SymbolTable::is_extern(string name)
{
     for(auto & el : symt_entry)
    {
        if(el.name != name)
            continue;

        if(el.mysec == "*UND*")
            return true;
    }

    return false;
}

int SymbolTable::symtable_size()
{
    return symt_entry.size();
}

bool SymbolTable::is_local(string name)
{
     for(auto & el : symt_entry)
    {
        if(el.name != name)
            continue;

        if(el.bind == "LOCAL")
            return true;
    }

    return false;
}

symdata& SymbolTable::at(int i)
{
    return symt_entry[i];
}

string SymbolTable::get_mysection(string name)
{
    for(auto & el : symt_entry)
    {
        if(el.name == name)
        {
            return el.mysec;
        }
    }

    return "nullptr";
}

void SymbolTable::redefine_symbol(string name, int value, string bind, string mysec)
{
    for(auto & el : symt_entry)
    {
        if(el.name != name)
            continue;

        el.value = value;
        el.bind = bind;
        el.mysec = mysec;
    }
}

int SymbolTable::get_symValue(string name)
{
    int ret = -1;

    for(symdata &el : symt_entry)
    {
        if(el.name != name)
            continue;
        else ret = el.value;
    }

    return ret;
}


void SymbolTable::st_print(stringstream &stream)
{
    stream  << endl << "Symbol table: " << endl;

    stream << left << setfill(' ') 
       << setw(15) << "NUM" 
       << setw(15) << "NAME" 
       << setw(15) << "VALUE" 
       << setw(15) << "BIND" 
       << setw(15) << "SECT_NDX" 
       << endl;
    

    int i = 0;
    for(auto & el : symt_entry)
    {
        if(el.name != el.mysec && el.name != "UND")
            continue;

       stream << left << setfill(' ') 
       << setw(15) << to_string(i)
       << setw(15) << el.name;
        if(el.value != -1)
        stream << left << setfill(' ') << setw(15) << el.value;
       else 
        stream << left << setfill(' ') << setw(15) << "";

        stream << left << setfill(' ')
        << setw(15) << el.bind 
        << setw(15) << el.mysec 
        << endl;
        i++;

    }
    for(auto & el : symt_entry)
    {
        if(el.name == el.mysec || el.name == "UND")
            continue;

       stream << left << setfill(' ') 
       << setw(15) << to_string(i)
       << setw(15) << el.name;

       if(el.value != -1)
        stream << left << setfill(' ') << setw(15) << el.value;
       else 
        stream << left << setfill(' ') << setw(15) << "";

        stream << left << setfill(' ')
        << setw(15) << el.bind 
        << setw(15) << el.mysec 
        << endl;
        i++;
    }

    stream << endl;
}