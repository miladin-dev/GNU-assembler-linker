#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <list>
#include <iomanip>
#include <algorithm>
#include <regex>
#include "../inc/linker.hpp"

using namespace std;

int main(int argc, char* argv[]){

    if(argc < 2)
    {
        cerr << "Error with argument counter!";
    }

    string output_fs;
    list<string> input_fs;
    vector<pair<string,string>> place_command;


    string linker = argv[1];

    if(linker.compare("linker") != 0){
        cerr << "Not linker command";
        return -1;
    }

    string linkable = argv[2];
    regex place("-place=(\\w+)@0x(\\w+)");
    smatch match;
    bool is_hex = false;

    if(linkable.compare("-linkable") == 0)
    {
        for(int i = 3; i < argc; i++)
        {
            string arg = argv[i];

            if(regex_search(arg, place))
                continue;

            if(arg.compare("-o") == 0)
            {
                i += 1;
                output_fs = argv[i];
            }
            else 
            {
                input_fs.push_back(argv[i]);    //lista ulaznih fajlova
            }
        }
    }
    else if(linkable == "-hex")
    {
        is_hex = true;
        for(int i = 3; i < argc; i++)
        {
            string arg = argv[i];

            if(regex_search(arg, match, place))
            {
                place_command.push_back(make_pair(match[1], match[2]));
                continue;
            }

            if(arg.compare("-o") == 0)
            {
                i += 1;
                output_fs = argv[i];
            }
            else 
            {
                input_fs.push_back(argv[i]);    //lista ulaznih fajlova
            }
        }
    }


    Linker linker_obj(output_fs, input_fs, place_command, is_hex);

}