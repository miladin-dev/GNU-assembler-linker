#include "../inc/secdata.hpp"

SectionData::SectionData(string name)
{
    this->name = name;
    section_offset = 0;
}

void SectionData::add_byte(int byte)
{
    if(byte > 255){
        cerr << "Byte length overload - Error." << endl;
        return;
    }

    data.push_back(byte);
}

vector<int> & SectionData::get_vector()
{
    return data;
    
}

string SectionData::get_name()
{
    return name;
}

void SectionData::set_sec_offset(int off)
{
    section_offset = off;
}

int SectionData::data_size()
{
    return data.size();
}

int SectionData::get_sec_offset()
{
    return section_offset;
}

void SectionData::print_sectiondata(stringstream &stream)
{
    if(data.empty())
        return;
 
    stream 
        << "Content of section "
        << "#" << name << ":"
    << endl;

    int counter = 0; 
    int addr = section_offset;

    stream << right << setfill(' ') << setw(7);
    for(int i = 0; i < 8; i++)
        stream << "0" << hex << i << " ";
    
    for(auto & el : data)
    {
        if(counter % 8 == 0){
            stream << endl;
            stream << right << setfill('0') << setw(4) << hex << (addr - addr%8) << ": ";
            addr += 8;
        }
        int r = section_offset % 8;
        if(counter == 0)
        {
            for(int i = 0; i < r; i++)
            {
                stream << "   ";
                counter++;
            }
        }
        counter++;

        stream << right <<  setfill('0') << setw(2) <<  hex << el << " ";
    }
    stream << endl << endl;
}