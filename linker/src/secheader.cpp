#include "../inc/secheader.hpp"
#include "../inc/linker.hpp"
#include <string>
#include <iomanip>
#include <iostream>



void SectionHeader::add(string name, int size, int offset)
{
    shdrdata shd;
    shd.idx = id++;
    shd.name = name;
    shd.size = size;
    shd.offset = offset;

    shdr_entry.push_back(shd);
    displacements.push_back(make_pair(name, 0));
}

bool SectionHeader::exists(string name)
{
    for(shdrdata & el : shdr_entry)
    {
        if(el.name.compare(name) == 0)
        {
            return true;
        }
    }

    return false;
}

void SectionHeader::set_offset(string name, int off)
{
    for(shdrdata & el : shdr_entry)
    {
        if(el.name.compare(name) == 0)
        {
            el.offset = off;
        }
    }
}

void SectionHeader::update_offset(vector<pair<string,string>> &place)
{
    int start = 0;
    for(shdrdata & sec : shdr_entry)
    {
        for(auto & p : place)
        {
            if(p.first == sec.name)
            {
                sec.offset = Linker::hexToInt(p.second);
                if(start < sec.offset + sec.size)
                    start = sec.offset + sec.size;
            }
        }
    }

    for(shdrdata & sec : shdr_entry)
    {
        bool found = false;
        for(auto & p : place)
        {
            if(p.first == sec.name)
            {
                found = true;
            }
        }

        if(!found)
        {
            sec.offset = start;
            start += sec.size;
        }
    }

}

int SectionHeader::get_size(string name)
{
    for(shdrdata & el : shdr_entry)
    {
        if(el.name.compare(name) == 0)
        {
            return el.size;
        }
    }
    return -1;
}

int SectionHeader::get_displacement(string name)
{
    for(int i = 0; i < displacements.size(); i++)
    {
        if(displacements[i].first == name)
            return displacements[i].second;
    }
    return -1;
}

void SectionHeader::pop_displacement(string name)
{
    for(int i = 0; i < displacements.size(); i++)
    {
        if(displacements[i].first == name){
            displacements.erase(displacements.begin() + i);
            return;
        }
    }
}

shdrdata& SectionHeader::at(int it)
{
    return shdr_entry[it];
}

int SectionHeader::get_offset(string name)
{
    for(auto & el : shdr_entry)
    {
        if(el.name == name)
            return el.offset;
    }
    return -1;
}

void SectionHeader::set_size(string name, int size)
{
    for(shdrdata & el : shdr_entry)
    {
        if(el.name.compare(name) == 0)
        {
            displacements.push_back(make_pair(name, el.size));
            el.size += size;
            return;
        }
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
       << setw(15) << hex << to_string(el.idx)
       << setw(15) << el.name
       << setw(15) << el.size
       << setw(15) << el.offset
       << endl;
    }

}