#include "../inc/symtable.hpp"
#include "../inc/assembler.hpp"
#include <iomanip>

int SymbolTable::st_ID = -1;

SymbolTable::SymbolTable()
{
    symdata sd_und;
    sd_und.id = ++st_ID;
    sd_und.name = "UND";
    sd_und.value = -1;
    sd_und.bind = -1;
    sd_und.mysec = -2;
    sd_und.is_equ = 0;
    sd_und.is_section = false;

    symt_entry.push_back(sd_und);
}

void SymbolTable::add(string name, int value, short bind, int ndx, bool equ)
{
    symdata sd;
    sd.id = ++st_ID;
    sd.name = name;
    sd.value = value;
    sd.bind = bind;
    sd.mysec = ndx;
    sd.is_equ = equ;
    sd.size = 0;

    if(ndx == -1)       //Adding section as symbol
    {
        sd.mysec = sd.id;
        sd.is_section = true;
    }

    symt_entry.push_back(sd);
}

int SymbolTable::get_id()
{
    return st_ID;
}

void SymbolTable::set_size(string name, int size)
{
    for(auto & el : symt_entry)
    {
        if(el.name != name)
            continue;
        

        el.size = size;
    }

    

}


bool SymbolTable::is_global(string name)
{
    for(auto & el : symt_entry)
    {
        if(el.name != name)
            continue;
        
        return el.bind;
        
    }

    return false;
}

bool SymbolTable::is_equ(string name)
{
    for(symdata &el : symt_entry)
    {
        if(el.name != name)
            continue;
        
        return el.is_equ;
    }

    return false;
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

int SymbolTable::get_symId(string name)
{
    int ret = -1;
    if(!symbol_exists(name))
    {
        cerr << "Symbol " << name << "does not exist." << endl;
        return -1;
    }
    for(symdata &el : symt_entry)
    {
        if(el.name != name)
            continue;
        else ret = el.id;
    }

    return ret;
}

int SymbolTable::get_sectionId(string symbol_name)
{
    int ret = -1;
    for(symdata &el : symt_entry)
    {
        if(el.name != symbol_name)
            continue;
        else ret = el.mysec;
    }

    return ret;
}

bool SymbolTable::symbol_exists(string name)
{
    for(symdata &el : symt_entry)
    {
        if(el.name == name)
            return true;
    }

    return false;
}

void SymbolTable::set_global(string name)
{
    for(symdata &el : symt_entry)
    {
        if(el.name == name){
            el.bind = GLOBAL;
            break;
        }
    }

}

string SymbolTable::get_symName(int id)
{
    for(symdata & el : symt_entry)
    {
        if(el.id == id)
        {
            return el.name;
        }
    }

    return "nullptr";
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
        if(el.id != el.mysec && el.id != 0)
            continue;

        stream << left << setfill(' ') 
        << setw(15) << to_string(i)
        << setw(15) << el.name;

        if(el.value != -1)
        stream << left << setfill(' ') << setw(15) << el.value;
       else 
        stream << left << setfill(' ') << setw(15) << "";
        
        if(el.bind == LOCAL)
        {
            stream << left << setfill(' ') << setw(15) << "LOCAL";
        }
        else {
            stream << left << setfill(' ') << setw(15) << "";
        }

        stream << left << setfill(' ') 
        << setw(15) << (el.id != 0 ? el.name : "*UND*")
        << endl;
       i++;
    }

    for(auto & el : symt_entry)
    {
        if(el.id == el.mysec || el.id == 0)
            continue;

        stream << left << setfill(' ') 
       << setw(15) << to_string(i)
       << setw(15) << el.name;

       if(el.value != -1)
        stream << left << setfill(' ') << setw(15) << el.value;
       else 
        stream << left << setfill(' ') << setw(15) << "";

       if(el.bind == GLOBAL)
       {
           stream << left << setfill(' ') << setw(15) << "GLOBAL";
       }
       else if(el.bind == LOCAL)
       {
           stream << left << setfill(' ') << setw(15) << "LOCAL";
       }
       else {
           stream << left << setfill(' ') << setw(15) << "";
       }
       stream 
       << setw(15) << print_mysection(el)
       << endl;
       i++;
    }

    stream << endl;

}

string SymbolTable::print_mysection(symdata& el)
{
    if(el.mysec == -2)
        return "*UND*";
    
    if(el.is_equ)
        return "*ABS*";

    for(auto & tmp : symt_entry)
    {
        if(tmp.id == el.mysec)
        {
            return tmp.name;
        }
            
    }

    return "nullptr";
}


RelTable::RelTable(string name)
{
    this->name = name;
}

void RelTable::push_entry(int off, string type, string value)
{
    reldata rd;
    rd.offset = off;
    rd.type = type;
    rd.value = value;
    relt_entry.push_back(rd);
}

void RelTable::print_reltable(stringstream &stream)
{
    if(relt_entry.empty())
        return;
    
    stream << "----------------- Relocation table of section  ";
    stream << name << " --------------" << endl;

    stream << left << setfill(' ') 
       << setw(15) << "OFFSET" 
       << setw(15) << "TYPE" 
       << setw(15) << "VALUE" 
    << endl;

    for(reldata& el : relt_entry)
    {
      stream << left << setfill(' ') 
       << setw(15) << hex << el.offset
       << setw(15) << el.type
       << setw(15) << el.value 
       << endl;
    }

}

string RelTable::get_name()
{
    return name;
}