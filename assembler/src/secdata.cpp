#include "../inc/secdata.hpp"
#include <iomanip>
#include <fstream>
#include <iostream>


SectionData::SectionData(string n)
{
    name = n;
}

void SectionData::add_word(int word)
{
    if(word > 65535){
        cerr << "Word length overload - Error." << endl;
        return;
    }

    data.push_back(word & 0xFF);
    data.push_back((word >> 8) & 0xFF);
}

void SectionData::add_byte(int byte)
{
    if(byte > 255){
        cerr << "Byte length overload - Error." << endl;
        return;
    }

    data.push_back(byte);

}

void SectionData::add_word_instr(int word)
{
     if(word > 65535){
        cerr << "Word length overload - Error." << endl;
        return;
    }

    data.push_back((word >> 8) & 0xFF);
    data.push_back(word & 0xFF);
}

void SectionData::print_sectionData(stringstream &stream)
{
   stream 
        << "Content of section "
        << "#" << name << ":"
    << endl;

    int counter = 0; 
    int addr = 0;

    stream << right << setfill(' ') << setw(7);
    for(int i = 0; i < 8; i++)
        stream << "0" << hex << i << " ";
    
    for(auto & el : data)
    {
        if(counter % 8 == 0){
            stream << endl;
            stream << right << setfill('0') << setw(4) << hex << addr << ": ";
            addr += 8;
        }
        counter++;
        stream << right <<  setfill('0') << setw(2) <<  hex << el << " ";
    }
    stream << endl << endl;


}



SectionHeader::SectionHeader()
{

}

void SectionHeader::add(string name, int size, int offset)
{
    shdrdata shd;
    shd.idx = id++;
    shd.name = name;
    shd.size = size;
    shd.offset = offset;

    shdr_entry.push_back(shd);
}

void SectionHeader::set_size(string name, int size)
{
    for(auto & el : shdr_entry)
    {
        if(el.name != name)
            continue;

        el.size = size;
    }
}


void SectionHeader::print_shdr(stringstream &stream)
{
    stream << "Section header:" << endl;
    stream << left << setfill(' ') 
       << setw(15) << "Idx" 
       << setw(15) << "Name" 
       << setw(15) << "Size" 
       << setw(15) << "Offset" 
    << endl;


    for(auto & el : shdr_entry)
    {
       stream << left << setfill(' ') 
       << setw(15) << to_string(el.idx)
       << setw(15) << el.name
       << setw(15) << hex << el.size
       << setw(15) << el.offset
       <<  endl;
    }

}