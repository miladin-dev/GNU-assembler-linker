#include "../inc/reltable.hpp"


RelTable::RelTable(string n)
{
    current_displ = 0;
    name = n;
}

void RelTable::push_entry(int offset, string type, string value)
{
    reldata rd;
    rd.offset = offset + current_displ;
    rd.type = type;
    rd.value = value;


    relt_entry.push_back(rd);
}


string RelTable::get_name()
{
    return name;
}

void RelTable::set_displ(int d)
{
    current_displ = d;
}

int RelTable::reltable_size()
{
    return relt_entry.size();
}

reldata& RelTable::at(int i)
{
    return relt_entry[i];
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

    stream << endl;

}
