
#include "../inc/assembler.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>


using namespace std;

Assembler::Assembler(string input_fs, string output_fs)
{
    input_fstring = input_fs;
    output_fstring = output_fs;
}  

Assembler::~Assembler()
{
    delete sym_table;
    delete sec_header;

    for(SectionData* sd : all_sec_data_list)
        delete sd;
    
    for(RelTable* rt : all_relt_list)
        delete rt;
}

void Assembler::run_assembler()
{
    ifstream input;
    ofstream output;
    stringstream stream;

    input.open(input_fstring);

    if(!input)
    {
        cerr << "Error opening input file, exiting...";
        return;
    }

    curr_sec.name = "";
    curr_sec.lcnt = 0;

    sym_table = new SymbolTable();
    sec_header = new SectionHeader();

    if(!first_pass(input))
    {
        cerr << "Unsuccessful..." << endl;
        return;
    }
    
    input.clear();
    input.seekg(0);
    
    if(!second_pass(input))
    {
        cerr << "Unsuccessful..." << endl;
        return;
    }

    output.open(output_fstring);
   
    if(!output)
    {
        cerr << "Error opening output file, exiting...";
        return; 
    }

    sec_header->print_shdr(stream);
    sym_table->st_print(stream);

    for(SectionData* sd : all_sec_data_list)
    {   
        sd->print_sectionData(stream);
        stream << endl;
    }

    for(RelTable* rt : all_relt_list)
    {
        rt->print_reltable(stream);
        stream << endl;
    }

    output << stream.str();
    //cout << stream.str();

    input.close();
    output.close();
}

bool Assembler::first_pass(ifstream &input)
{
    string line;
    for(line; getline(input, line);){
        if(!line.empty() && !is_commented(line)){

            if(is_end(line) && edit_global_directives()){
                if(!curr_sec.name.empty()){
                    sec_header->set_size(curr_sec.name, curr_sec.lcnt);
                    sym_table->set_size(curr_sec.name, curr_sec.lcnt);
                }
                return true;
            }
            if(fp_directive(line))
                continue;
            if(fp_label(line))
                continue;
            if(fp_label_extended(line))
                continue;
            if(fp_instruction(line))
                continue;
            
            cerr << "Error with line '" << line << "'.Exiting..." << endl;
            return false;
        }
    }

    return false;
    
}

bool Assembler::fp_directive(string &line)
{
    if(!is_directive(line))
        return false;

    regex rxp_dir_name("^\\s*\\." "(" RXP_ALF ")" "\\s+.*$", std::regex_constants::icase);
    regex one_directive(RXP_START "\\." "(" RXP_ALF ")" RXP_END, std::regex_constants::icase); //.end .text itd
    smatch match;
        
    if(regex_search(line, match, rxp_dir_name))
    {
        string directive = toLowerCase(match.str(1));
        regex regExp_op;
        smatch operands;

        if(directive == "global")
        {
           global_directives.push_back(line);
           return true;
        }
        //.EQU
        if(directive == "equ")
        {
            regExp_op = (RXP_START "\\.\\w+\\s+(\\w+)\\s*,\\s*(\\d+)" RXP_END);
            if(regex_search(line, operands, regExp_op))
            {
                string symbol = operands[1];
                int value = stoi(operands[2]);
                sym_table->add(symbol, value, LOCAL, curr_sec.sec_id, IS_EQU);
                return true;
            }
            else    //if its .equ a, 0xFF
            {
                regExp_op = (RXP_START "\\.\\w+\\s*(\\w+)\\s*,\\s*0[xX]([A-Za-z0-9]+)" RXP_END);
                if(regex_search(line, operands, regExp_op))
                {
                    string symbol1 = operands[1];
                    int symbol2 = hexToInt(operands[2]);
                    sym_table->add(symbol1, symbol2, LOCAL, curr_sec.sec_id, IS_EQU);
                    return true;
                }
                else 
                {
                    cerr << "ERROR with .equ directive. Unable to assemble it." << endl;
                    return false;
                }
            }
        }
        //.EXTERN
        if(directive == "extern")
        {
            regExp_op = (RXP_START "\\.\\w+\\s+([^#]+)" RXP_END);    //.extern (a, bd, c)-> this is matched

            if(regex_search(line, operands, regExp_op))
            {
                //split => a, bd, c
                string to_split = operands.str(1);
                vector<string> tokens;
                if(separate_symbols(tokens, to_split))
                {
                    for(auto & el : tokens)
                    {
                        sym_table->add(el, -1, UND, UND, false);
                    }
                        return true;
                }
                else {
                    cerr << "ERROR with .extern directive. Unable to assemble it." << endl;
                    return false;
                }
            }
            
        }
        //.section
        if(directive == "section")
        {
            regExp_op = (RXP_START "\\.\\w+\\s+(\\w+)" RXP_END);

            bool format_right = regex_search(line, operands, regExp_op);
            if(format_right)
            {
                string section_name = operands[1];

                if(!curr_sec.name.empty())
                {
                    sec_header->set_size(curr_sec.name, curr_sec.lcnt);
                    sym_table->set_size(curr_sec.name, curr_sec.lcnt);
                }

                curr_sec.lcnt = 0;
                curr_sec.name = section_name;

                sym_table->add(section_name, curr_sec.lcnt, LOCAL, -1, false);
                sec_header->add(curr_sec.name, 0, 0);
                
                curr_sec.sec_id = SymbolTable::get_id();

                return true;
            }
            else {
                cerr << "ERROR with .section directive. Unable to assemble it." << endl;
                return false;
            }
        }
        //.word
        if(directive == "word")
        {
            
            regExp_op = (RXP_START "\\.\\w+\\s+([^#]+)" RXP_END);    //.word a, bb, c
            if(regex_search(line, operands, regExp_op))
            {
                string to_split = operands[1];
                vector<string> tokens;
                if(separate_symbols(tokens, to_split))
                {
                    for(auto & el : tokens)
                    {
                        curr_sec.lcnt += 2;
                    }

                    return true;
                }
                else{
                    cerr << "ERROR with .word directive. Unable to assemble it." << endl;
                    return false;
                }
            }

        }
        //.skip
        if(directive == "skip")
        {
            regExp_op = (RXP_START "\\.\\w+\\s+(\\d+)" RXP_END);
            regex rxp_skip_hex(RXP_START "\\.\\w+\\s+" RXP_HEX_MATCH RXP_END);

            if(regex_search(line, operands, regExp_op))
            {
                int bytes_to_skip = stoi(operands[1]);
                curr_sec.lcnt += bytes_to_skip;
                return true;
            }
            if(regex_search(line, operands, rxp_skip_hex))
            {
                int bytes_to_skip = hexToInt(operands[1]);
                curr_sec.lcnt += bytes_to_skip;
                return true;
            }
        }
    }

    return false;
}

bool Assembler::fp_label(string &line)
{
    regex rxp_label(RXP_START "(\\w+)\\s*:" RXP_END);
    smatch m_symbol;
    bool is_label = regex_search(line, m_symbol, rxp_label);
    
    if(!is_label)
        return false;

    string symbol = m_symbol[1];


    sym_table->add(symbol, curr_sec.lcnt, LOCAL, curr_sec.sec_id, false);
    
    return true;
}


bool Assembler::fp_label_extended(string &line)
{
    regex rxp_label(RXP_START "(\\w+)\\s*:(.*)$");
    smatch match;

    if(regex_search(line, match, rxp_label))
    {
        string symbol = match[1];
        sym_table->add(symbol, curr_sec.lcnt, LOCAL, curr_sec.sec_id, false);
        string rest = match[2];

        if(fp_instruction(rest))
            return true;
        if(fp_directive(rest))
            return true;
        else 
            return false;
    }
    return false;
}


bool Assembler::fp_instruction(string &line)
{

    InstructionInfo info = get_instr_info(line, 1);
    int size = info.size;
    if(size != -1)
        return true;
    else 
        return false;
   
}

bool Assembler::is_end(string &line)
{
    regex end(RXP_START ".end" RXP_END, std::regex_constants::icase);

    if(regex_search(line, end))
        return true;
    else return false;
}

bool Assembler::edit_global_directives()
{
    for(string &line : global_directives)
    {
        regex rxp(RXP_START "\\.\\w+\\s+([^#]+)" RXP_END);
        smatch operands;

        if(regex_search(line, operands, rxp))
        {
            string to_split = operands[1];
            vector<string> tokens;

                if(separate_symbols(tokens, to_split))
                { 
                    for(string symbol : tokens)
                    {
                        if(sym_table->symbol_exists(symbol))
                        {
                            sym_table->set_global(symbol);
                        }
                        else
                        {
                            cerr << "Directive .global used symbol that does not exist." << endl; 
                            return false;
                        }
                    }
                }
                else 
                    return false;
            
        }
    }

    return  true;
}

/* ------------------------------------------------------------------------------------ */
/*                                  SECOND PASS                                         */
/* ------------------------------------------------------------------------------------ */


bool Assembler::second_pass(ifstream &input)
{
    string line;
    for(line; getline(input, line);){
        if(!line.empty() && !is_commented(line))
        {   
            if(is_end(line))
                return true;
            if(sp_directive(line))
                continue;
            if(sp_label(line))
                continue;
            if(sp_label_extended(line))
                continue;
            if(sp_instruction(line))
                continue;

            cout << "Could not finish second pass - exiting..." << endl;
            return false;
        }
    }

    return false;
}

bool Assembler::sp_label(string &line)
{
    regex label(RXP_START RXP_ALF "\\s*:" RXP_END);

    if(regex_search(line, label))
        return true;
    else 
        return false;
}

bool Assembler::sp_label_extended(string &line)
{
    regex rxp_label(RXP_START "(\\w+)\\s*:(.*)$");
    smatch match;

    if(regex_search(line, match, rxp_label))
    {
        string rest = match[2];

        if(sp_instruction(rest))
            return true;
        if(sp_directive(rest))
            return true;
        else 
            return false;
    }

    return false;
}

bool Assembler::sp_directive(string &line)
{
    if(!is_directive(line))
        return false;

    regex rxp_dir_name("^\\s*\\.(\\w+)\\s+.*$", std::regex_constants::icase);
    regex one_directive(RXP_START "\\." "(" RXP_ALF ")" RXP_END, std::regex_constants::icase); //.end .text itd
    smatch match;
        
    if(regex_search(line, match, rxp_dir_name))
    {
        string directive = toLowerCase(match.str(1));
        regex regExp_op;
        smatch operands;

        if(directive == "section")
        {
            regExp_op = (RXP_START "\\.\\w+\\s+(\\w+)" RXP_END);

            if(regex_search(line, operands, regExp_op))
            {
                string sec_name = operands[1];
                
                curr_sec.lcnt = 0;
                curr_sec.name = sec_name;
                curr_sec.sec_id = sym_table->get_symId(sec_name);

                curr_sec.data = new SectionData(sec_name);
                all_sec_data_list.push_back(curr_sec.data);

                curr_sec.relt = new RelTable(sec_name);
                all_relt_list.push_back(curr_sec.relt);

                return true;
            }
            
            return false;
        }
        if(directive == "global")
        {
            return true;
        }
        if(directive == "skip")
        {
            regex RXP_SKIP_LITERAL(RXP_START "\\.\\w+\\s+(" RXP_DIGIT ")" RXP_END);
            regex RXP_SKIP_HEX(RXP_START "\\.\\w+\\s+" RXP_HEX_MATCH RXP_END);

            if(regex_search(line, operands, RXP_SKIP_LITERAL))
            {
                int bytes_to_skip = stoi(operands[1]);
                curr_sec.lcnt += bytes_to_skip;
                for(int i = 0; i < bytes_to_skip; i++)
                    curr_sec.data->add_byte(0);


                return true;
            }
            if(regex_search(line, operands, RXP_SKIP_HEX))
            {
                int bytes_to_skip = hexToInt(operands[1]);
                curr_sec.lcnt += bytes_to_skip;
                for(int i = 0; i < bytes_to_skip; i++)
                    curr_sec.data->add_byte(0);

                return true;
            }

            return false;
        }
        if(directive == "word")
        {
            regex RXP_WORD_LITERAL(RXP_START "\\.\\w+\\s+(" RXP_DIGIT ")" RXP_END);
            regex RXP_WORD_LITERAL_HEX (RXP_START "\\.\\w+\\s+0[xX]([A-Za-z0-9]+)" RXP_END);
            regex RXP_WORD_SYMBOLS (RXP_START "\\.\\w+\\s+([^#]+)" RXP_END);  //OVAJ CE MI Mecovati i za literal_hexa

            //If its .word 5 - literal
            if(regex_search(line, operands, RXP_WORD_LITERAL))
            {
                int literal = stoi(operands[1]);
                curr_sec.data->add_word(literal);
                curr_sec.lcnt += 2;

                return true;
            }
            //If its .word 0x55
            else if(regex_search(line, operands, RXP_WORD_LITERAL_HEX))
            {
                string hex_str = operands[1];
                int hex_value = hexToInt(hex_str);
                curr_sec.data->add_word(hex_value);
                curr_sec.lcnt += 2;

                return true;
            }
            //If its .word isr_timer, b_lol
            else if(regex_search(line, operands, RXP_WORD_SYMBOLS))
            {
                string to_split = operands[1];
                vector<string> tokens;

                if(separate_symbols(tokens, to_split))
                { 
                    for(string symbol_str : tokens)
                    {
                        if(!sym_table->symbol_exists(symbol_str))
                        {
                            cerr << "Simbol " << symbol_str << " does not exist." << endl;
                            return false;
                        }
                        if(sym_table->is_equ(symbol_str))
                        {
                            int value = sym_table->get_symValue(symbol_str);    
                            curr_sec.data->add_word(value);
                            //no relocation when symbol is EQU
                        }
                        else 
                        {
                            if(sym_table->is_global(symbol_str))
                            {
                                int symbol_id = sym_table->get_symId(symbol_str);
                                curr_sec.data->add_word(0);
                                curr_sec.relt->push_entry(curr_sec.lcnt, "R_DATA_VN_16", symbol_str);
                            }
                            else 
                            {
                                int symbol_value = sym_table->get_symValue(symbol_str);
                                int section_of_symbol = sym_table->get_sectionId(symbol_str);
                                string section = sym_table->get_symName(section_of_symbol);
                                curr_sec.data->add_word(symbol_value);
                                curr_sec.relt->push_entry(curr_sec.lcnt, "R_DATA_VN_16", section);
                            }
                        }

                        curr_sec.lcnt += 2;
                    }

                    return true;
                }
                else return false;
            }
        }
        if(directive == "extern")
        {
            return true;
        }
        if(directive == "equ")
        {
            return true;
        }
    }

    return false;
}

bool Assembler::sp_instruction(string &line)
{
    InstructionInfo instr_info = get_instr_info(line, 2);
    int inst_size = instr_info.size;

    if(inst_size == -1) 
        return false;

    if(inst_size == 1)
    {
        curr_sec.data->add_byte(instr_info.byte[0]);  
    }
    if(inst_size == 2)
    {
        curr_sec.data->add_byte(instr_info.byte[0]);
        curr_sec.data->add_byte(instr_info.byte[1]);
    }
    
    if(inst_size == 3)
    {
        curr_sec.data->add_byte(instr_info.byte[0]);
        curr_sec.data->add_byte(instr_info.byte[1]);
        curr_sec.data->add_byte(instr_info.byte[2]);
    }

    if(inst_size == 5)
    {
        curr_sec.data->add_byte(instr_info.byte[0]);
        curr_sec.data->add_byte(instr_info.byte[1]);
        curr_sec.data->add_byte(instr_info.byte[2]);
        curr_sec.data->add_word_instr(instr_info.data_word);

        if(sym_table->is_equ(instr_info.symbol))
            return true;

        if(instr_info.is_symbol && instr_info.pc == PCABS && sym_table->is_global(instr_info.symbol))
            curr_sec.relt->push_entry(curr_sec.lcnt - 2, "R_VN_16", instr_info.symbol);

        if(instr_info.is_symbol && instr_info.pc == PCABS && !sym_table->is_global(instr_info.symbol))      //example for jmp *myStart myStart is local
            curr_sec.relt->push_entry(curr_sec.lcnt - 2, "R_VN_16", sym_table->get_symName(sym_table->get_sectionId(instr_info.symbol)));

        if(instr_info.is_symbol && instr_info.pc == PCREL && !sym_table->is_global(instr_info.symbol))
            curr_sec.relt->push_entry(curr_sec.lcnt - 2, "R_VN_PC_16", sym_table->get_symName(sym_table->get_sectionId(instr_info.symbol)));
    
        if(instr_info.is_symbol && instr_info.pc == PCREL && sym_table->is_global(instr_info.symbol))
            curr_sec.relt->push_entry(curr_sec.lcnt - 2, "R_VN_PC_16", instr_info.symbol);
    }

    return true;
}

InstructionInfo Assembler::get_instr_info(string &line, int check_pass)
{
    regex rxp_push_pop(PUSH_POP, std::regex_constants::icase);

    //1 byte instructions
    regex rxp_oneb(ONEB_STR, std::regex_constants::icase);

    // 2 bytes instructions
    regex rxp_twob_rdir(REG_DIR_TWOB_STR, std::regex_constants::icase);

    // 3 bytes instructions
    regex rxp_threeb_jmp(THREEB_JMP);
    regex rxp_threeb_data(THREEB_DATA, std::regex_constants::icase);

    // 5 bytes instructions
    regex rxp_jmp_imm(JMP_IMM, std::regex_constants::icase);
    regex rxp_jmp_mem(JMP_MEM, std::regex_constants::icase);
    regex rxp_jmp_rid(JMP_RID, std::regex_constants::icase);

    regex rxp_ldst_imm(LDST_IMM, std::regex_constants::icase);
    regex rxp_ldst_mem(LDST_MEM, std::regex_constants::icase);
    regex rxp_ldst_rid(LDST_RID, std::regex_constants::icase);


    smatch match;
    InstructionInfo in_fo;
    in_fo.size = -1;
    string instruction;


    if(regex_search(line, match, rxp_push_pop))
    {
        in_fo.instr = toLowerCase(match[1]);
        in_fo.regD = register_alias(match[2]);
        string upbits;

        if(in_fo.instr.compare("push") == 0)
            upbits = "1";
    
        if(in_fo.instr.compare("pop") == 0)
            upbits = "4";


        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.regS = "6";   // sp = r6;
        in_fo.byte[1] = hexToInt(in_fo.regD + in_fo.regS);
        in_fo.byte[2] = hexToInt(upbits + "2"); // 2 = regind

        curr_sec.lcnt += 3;
        in_fo.size = 3;

    }
    //REG DIR ONE BYTE HALT|IRET|RET
    else if(regex_search(line, match, rxp_oneb))
    {
        in_fo.instr = toLowerCase(match[1]);
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));

        in_fo.size = 1;
        curr_sec.lcnt += 1;
    }


    // ----------- ADD XOR XCHG .... INT -------
    else if(regex_search(line, match, rxp_twob_rdir))
    {
        int type = match_index(match);
        if(type == 1)
        {
            in_fo.regS = register_alias(match[type + 2]);
        }
        else 
        {
            in_fo.regS = "0";
        }

        in_fo.regD = register_alias(match[type + 1]);
        in_fo.instr = toLowerCase(match[type]);
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.byte[1] = hexToInt(in_fo.regD + in_fo.regS);

        in_fo.size = 2;
        curr_sec.lcnt += 2;
    }
    // ----------- JMP REG DIR, REG IND -------
    else if(regex_search(line, match, rxp_threeb_jmp))
    {
        int use = match_index(match);

        in_fo.instr = toLowerCase(match[use]);
        in_fo.regD = "F";
        in_fo.regS = register_alias(match[use + 1]);
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.byte[1] = hexToInt(in_fo.regD + in_fo.regS);
        in_fo.byte[2] = (use == 1 ? REG_DIRECT : REG_IND);

        in_fo.size = 3;
        curr_sec.lcnt += 3;
    }
    //LDR|STR 3 BYTES REGDIR & REG IND
    else if(regex_search(line, match, rxp_threeb_data))
    {
        int type = match_index(match);

        in_fo.instr = toLowerCase(match[type]);
        in_fo.regD = register_alias(match[type + 1]);
        in_fo.regS = register_alias(match[type + 2]);

        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.byte[1] = hexToInt(in_fo.regD + in_fo.regS);
        in_fo.byte[2] = (type == 1 ? REG_DIRECT : REG_IND);

        in_fo.size = 3;
        curr_sec.lcnt += 3;
    }
    
    // ----------- LDR STR IMMEDIATE -------
    //  ldr r1, $myStart
    //  ldr r1, $10
    //  ldr r1, $0x10
    else if(regex_search(line, match, rxp_ldst_imm))
    {
        if(check_pass == 1) //provera zbog ovog ispisa symbol does not exits u prvom prolazu
        {
            in_fo.size = 5;
            curr_sec.lcnt += 5;
            
            return in_fo;
        }

        int type = match_index(match);
        
        if(type == 1)
        {
            in_fo.pc = PCABS;
            in_fo.is_symbol = true;
            in_fo.symbol = match[type + 2];

            if(!sym_table->symbol_exists(in_fo.symbol))
            {
                cerr << "Symbol '" << in_fo.symbol << "' does not exist." << endl;
                return in_fo;
            }

            sym_table->is_global(in_fo.symbol) ? 
                in_fo.data_word = 0 : in_fo.data_word = sym_table->get_symValue(in_fo.symbol);
        }
        else if(type == 4)
        {
            in_fo.literal = stoi(match[type + 2]);
            in_fo.data_word = in_fo.literal;
        }
        else 
        {
            in_fo.literal = hexToInt(match[type + 2]);
            in_fo.data_word = in_fo.literal;
        }

        in_fo.instr = toLowerCase(match[type]);
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.regD = register_alias(match[type + 1]);
        in_fo.regS = "0";
        in_fo.byte[1] = hexToInt(in_fo.regD + in_fo.regS);
        in_fo.byte[2] = IMMED;
        in_fo.size = 5;
        curr_sec.lcnt += 5;
    }
    
    // ----------- LDR STR MEMORY -------
    //  ldr r1, myStart # abs
    //  ldr r1, %myStart # pcrel
    //  ldr r1, 10
    //  ldr r1, 0x10
    else if(regex_search(line, match, rxp_ldst_mem))
    {
        if(check_pass == 1)
        {
            in_fo.size = 5;
            curr_sec.lcnt += 5;
            return in_fo;
        }

        int type = match_index(match);

        if(type == 1)
        {
            in_fo.pc = PCABS;
            in_fo.is_symbol = true;
            in_fo.symbol = match[type + 2];

            if(!sym_table->symbol_exists(in_fo.symbol))
            {
                cerr << "Symbol '" << in_fo.symbol << "' does not exist." << endl;
                return in_fo;
            }

            sym_table->is_global(in_fo.symbol) ? 
                    in_fo.data_word = 0 : in_fo.data_word = sym_table->get_symValue(in_fo.symbol);
        
        }
        else if(type == 4)
        {
            in_fo.pc = PCREL;
            in_fo.is_symbol = true;
            in_fo.symbol = match[type + 2];

            if(!sym_table->symbol_exists(in_fo.symbol))
            {
                cerr << "Symbol '" << in_fo.symbol << "' does not exist." << endl;
                return in_fo;
            }

            sym_table->is_global(in_fo.symbol) ? 
                    in_fo.data_word = -2 : in_fo.data_word = sym_table->get_symValue(in_fo.symbol) - 2;
            
        }
        else if(type == 7)
        {
            in_fo.literal = stoi(match[type + 2]);
            in_fo.data_word = in_fo.literal;
        }
        else
        {
            in_fo.literal = hexToInt(match[type + 2]);
            in_fo.data_word = in_fo.literal;
        }

        in_fo.instr = toLowerCase(match[type]);
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.regD = register_alias(match[type + 1]);
        in_fo.regS = "0";
        in_fo.byte[2] = MEM;
        if(type == 4)
        {
            in_fo.regS = "7";
            in_fo.byte[2] = REG_IND_DISPL;
        }
        in_fo.byte[1] = hexToInt(in_fo.regD + in_fo.regS);
        
        in_fo.size = 5;
        curr_sec.lcnt += 5;
    }
    
    // ----------- LDR STR REG_IND_DISPLACEMENT -------
    //  ldr r1, [r4 + a]
    //  ldr r1, [r4 + 5]
    //  ldr r1, [r4 + 0x5]
    else if(regex_search(line, match, rxp_ldst_rid))
    {
        if(check_pass == 1)
        {
            in_fo.size = 5;
            curr_sec.lcnt += 5;
            return in_fo;
        }
        
        int type = match_index(match);
            
        if(type == 1)
        {
            in_fo.pc = PCABS;
            in_fo.is_symbol = true;
            in_fo.symbol = match[type + 4];

            if(!sym_table->symbol_exists(in_fo.symbol))
            {
                cerr << "Symbol '" << in_fo.symbol << "' does not exist." << endl;
                return in_fo;
            }

            sym_table->is_global(in_fo.symbol) ? 
                    in_fo.data_word = 0 : in_fo.data_word = sym_table->get_symValue(in_fo.symbol);

        }
        else if(type == 6)
        {
            
            in_fo.literal = stoi(match[type + 4]);
            if(match[type + 3] == '-')
                in_fo.literal = -in_fo.literal;
            
            in_fo.data_word = in_fo.literal;
        }
        else
        {
            in_fo.literal = hexToInt(match[type + 4]);
             if(match[type + 3] == '-')
                in_fo.literal = -in_fo.literal;
            in_fo.data_word = in_fo.literal;
        }

        in_fo.instr = toLowerCase(match[type]);
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.regD = register_alias(match[type + 1]);
        in_fo.regS = register_alias(match[type + 2]);
        in_fo.byte[1] = hexToInt(in_fo.regD + in_fo.regS);
        in_fo.byte[2] = REG_IND_DISPL;
       
        in_fo.size = 5;
        curr_sec.lcnt += 5;
    }

    // ------------- JMP IMMEDIATE --------------
    // jeq a  - ABS
    // jeq %a  -PCrel
    // jeq 55
    // jeq 0x55
    else if(regex_search(line, match, rxp_jmp_imm))
    {
        if(check_pass == 1)
        {
            in_fo.size = 5;
            curr_sec.lcnt += 5;
            return in_fo;
        }
        

        int type = match_index(match);
        int tmp = type;

        if(type == 1)
        {
            in_fo.pc = PCABS;
            in_fo.is_symbol = true;
            in_fo.symbol = match[++tmp];

            if(!sym_table->symbol_exists(in_fo.symbol))
            {
                cerr << "Symbol '" << in_fo.symbol << "' does not exist." << endl;
                return in_fo;
            }

            if(sym_table->is_global(in_fo.symbol))
            {
                in_fo.data_word = 0;   
            }
            else 
            {
                in_fo.data_word = sym_table->get_symValue(in_fo.symbol);
            }
        }
        else if(type == 3)
        {
            in_fo.pc = PCREL;
            in_fo.is_symbol = true;
            in_fo.symbol = match[++tmp];

            if(!sym_table->symbol_exists(in_fo.symbol))
            {
                cerr << "Symbol '" << in_fo.symbol << "' does not exist." << endl;
                return in_fo;
            }

            if(sym_table->is_global(in_fo.symbol))
            {
                in_fo.data_word = -2;   
            }
            else 
            {
                in_fo.data_word = sym_table->get_symValue(in_fo.symbol) - 2;    //-2 is field size
            }

        }
        else if(type == 5)  //je 55
        {
            in_fo.literal = stoi(match[++tmp]);
            in_fo.data_word = in_fo.literal;
        }
        else    //jeq 0x55
        {
            in_fo.literal = hexToInt(match[++tmp]);
            in_fo.data_word = in_fo.literal;
        }

        in_fo.instr = toLowerCase(match[type]);
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.byte[1] = hexToInt("F0");
        in_fo.byte[2] = IMMED;
        if(type == 3)
        {
            in_fo.byte[1] = hexToInt("F7");
            in_fo.byte[2] = REG_DIR_DISPL;
        }
        in_fo.size = 5;
        curr_sec.lcnt += 5;
    }
    
    // ------------- JMP MEMORY ------------------
    // jmp *52
    // jmp *0x52
    // jmp *mile
    else if(regex_search(line, match, rxp_jmp_mem))
    {
        if(check_pass == 1)
        {
            in_fo.size = 5;
            curr_sec.lcnt += 5;
            return in_fo;
        }
        
        int type = match_index(match);
        int pom = type;

        if(type == 1)   // jmp *50
        {
            in_fo.literal = stoi(match[++pom]);
            in_fo.data_word = in_fo.literal;
        }
        else if(type == 3)  // jmp *0x52
        {
            in_fo.literal = hexToInt(match[++pom]);
            in_fo.data_word = in_fo.literal;
        }
        else    // jmp *myStart
        {
            in_fo.is_symbol = true;
            in_fo.symbol = match[++pom];

            if(!sym_table->symbol_exists(in_fo.symbol))
            {
                cerr << "Symbol '" << in_fo.symbol << "' does not exist." << endl;
                return in_fo;
            }
            
            if(sym_table->is_global(in_fo.symbol))
            {
                in_fo.data_word = 0;   
            }
            else 
            {
                in_fo.data_word = sym_table->get_symValue(in_fo.symbol);
            }
        }

        in_fo.instr = toLowerCase(match[type]);
        in_fo.pc = PCABS;
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.byte[1] = hexToInt("F0");
        in_fo.byte[2] = MEM;
        in_fo.size = 5;
        curr_sec.lcnt += 5;
        
    }

    // ------------- JMP REG IND DISPLACEMENT ------------------
    // jmp *[r5 + myStart]
    // jmp *[r4 + 5]
    // jmp *[r4 + 0x22] 
    else if(regex_search(line, match, rxp_jmp_rid))
    {
        if(check_pass == 1)
        {
            in_fo.size = 5;
            curr_sec.lcnt += 5;
            return in_fo;
        }
        

        int type = match_index(match);
        int pom = type;

        if(type == 1)           // jmp *[r5 + myStart]
        {
            //ako je lokalni izvrsi relokaciju u odnosu na sekciju, a unesi value symbola
            //globalni unosis 0, a relokacija u odnosu na simbol
            in_fo.pc = PCABS;
            in_fo.is_symbol = true;
            in_fo.symbol = match[pom + 3];

            if(!sym_table->symbol_exists(in_fo.symbol))
            {
                cerr << "Symbol '" << in_fo.symbol << "' does not exist." << endl;
                return in_fo;
            }

            sym_table->is_global(in_fo.symbol) ? in_fo.data_word = 0 : in_fo.data_word = sym_table->get_symValue(in_fo.symbol);

        }
        else if(type == 5)  // r5 + 5
        {
            in_fo.literal = stoi(match[type + 3]);

            if(match[type + 2] == '-')
                in_fo.literal = -in_fo.literal;

            in_fo.data_word = in_fo.literal;
        }
        else 
        {
            in_fo.literal = hexToInt(match[type + 3]);
            if(match[type + 2] == '-')
                in_fo.literal = -in_fo.literal;
            in_fo.data_word = in_fo.literal;
        }

        in_fo.instr = toLowerCase(match[type]);
        in_fo.regD = "F";
        in_fo.regS = register_alias(match[type + 1]);
        in_fo.byte[0] = hexToInt(get_opcode(in_fo.instr));
        in_fo.byte[1] = hexToInt(in_fo.regD + in_fo.regS);
        in_fo.byte[2] = REG_IND_DISPL;
        in_fo.size = 5;
        curr_sec.lcnt += 5; 
    }


    return in_fo;
}

int Assembler::match_index(smatch match)
{
    string check;
    int use = 0;
    for(int i = 1; i < match.size(); i++)
    {
        check = match.str(i);
        if(!check.empty()){
            use = i;
            break;
        }
    }

    return use;
}

string Assembler::toLowerCase(string data)
{
    std::transform(data.begin(), data.end(), data.begin(),
    [](unsigned char c){ return std::tolower(c); });

    return data;
}

int Assembler::hexToInt(string to_convert)
{
    const char* cstr = to_convert.c_str();
    int symbol2 = (int)strtol(cstr, NULL, 16);
 
    return symbol2;
}

string Assembler::get_opcode(string instr)
{
    return instr_codes.find(instr)->second;
}

string Assembler::register_alias(string regg)
{
    regex check("^r([0-7])|(sp)|(pc)|(psw)$");
    smatch match;

    if(regex_search(regg, match, check))
    {
        int type = match_index(match);
        if(type == 1)
            return match[1];

        if(type == 2)
            return "6";
        if(type == 3)
            return "7";
        if(type == 4)
            return "8";
    }

    return "";
}

bool Assembler::separate_symbols(vector<string> &tokens, string &to_split)
{
    stringstream ss(to_split);
    string buf;
    regex rxp_one_symbol("^\\s*(\\w+)\\s*$");
    smatch symbols;

    while(getline(ss, buf, ','))
    {
        if(regex_search(buf, symbols, rxp_one_symbol))
        {
            string sym = symbols[1];
            tokens.push_back(sym);
            continue;
        }
        else {
            return false;
        }
    }

    return true;
}

bool Assembler::is_directive(string &line)
{
    regex reg(RXP_START "\\.[^.#;@]+" RXP_END);

    if(regex_search(line, reg))
    {
        return true;
    }
    else {
        return false;
    }
}

bool Assembler::is_commented(string &line)
{
    regex is_blank_space("^\\s+$");
    regex rxp_comm("^\\s*#.*$");
    if(regex_search(line, rxp_comm) || regex_search(line, is_blank_space)) 
        return true;
    else return false;
}