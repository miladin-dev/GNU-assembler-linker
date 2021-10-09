#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <algorithm>
#include "../inc/assembler.hpp"

using namespace std;

int main(int argc, char* argv[]){


    string assm = argv[1];
    if(argc < 2 || assm != "assembler")
    {
        cerr << "Error with argument input...";
        return 1;
    }

    string input_fs;
    string output_fs;

    for(int i = 2; i < argc; i++){
        string arg = argv[i];
        if(arg.compare("-o") == 0)
        {
            i++;
            output_fs = argv[i];
        }
        else 
        {
            if(input_fs.empty())
            {
                input_fs = argv[i];
            }
        }
    }
    
   
    Assembler assembler(input_fs, output_fs);
    assembler.run_assembler();
}