#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H

#include <string>
#include <stdio.h>
#include <vector>
#include <map>
#include <regex>
#include "symtable.hpp"
#include "secdata.hpp"

#define RXP_START "^\\s*"
#define RXP_END "\\s*(#.*)?$"
#define RXP_REG "r[0-7]"
#define RXP_REG_MATCH "(r[0-7]|sp|pc|psw)"
#define RXP_HEX "0[xX][A-Za-z0-9]+"
#define RXP_HEX_MATCH "0[xX]([A-Za-z0-9]+)"
#define RXP_ALF "[A-Za-z_]+"
#define RXP_DIGIT "[0-9]+"
#define RXP_REG_IND_DISPL_D "\\[\\s*" RXP_REG_MATCH "\\s*(\\+|-)\\s*" "(" RXP_DIGIT ")" "\\s*\\]" // [r5 + 5]
#define RXP_REG_IND_DISPL_A "\\[\\s*" RXP_REG_MATCH "\\s*(\\+|-)\\s*" "(" RXP_ALF ")" "\\s*\\]" // [ r5 + a ] 
#define RXP_REG_IND_DISPL_H "\\[\\s*" RXP_REG_MATCH "\\s*(\\+|-)\\s*" RXP_HEX_MATCH "\\s*\\]" // [ r5 + 0x55 ] 
#define RXP_REG_IND "\\[\\s*" RXP_REG_MATCH "\\s*\\]"
#define AR_OP "7"
#define LOG_OP "8"
#define SH_OP "9"
#define JMP_OP "5"
#define PCABS 0
#define PCREL 1
#define IMMED 0 
#define REG_DIRECT 1 
#define REG_IND 2 
#define REG_IND_DISPL 3 
#define MEM 4
#define REG_DIR_DISPL 5
#define displ_inst 1
#define displ_reg 4
#define EXTERN -3

using namespace std;

typedef struct 
{
    int sec_id;
    unsigned int lcnt;
    string name;
    SectionData* data;
    RelTable* relt;

} Section;

typedef struct
{
    string instr;
    int op_code;
    int up_bits;
    int size = -1;
    string regD;
    string regS;
    int adrr_type;
    string symbol;
    int pc;
    bool is_symbol = false;
    int literal;

    int byte[3];
    int data_word;
    int byte1;
    int byte2;
    int byte3;
    int byte4;
    int byte5;

} InstructionInfo;

class Assembler{
public:
    Assembler(string input, string output);
    ~Assembler();
    bool is_directive(string &line);
    bool is_commented(string &line);
    
    bool first_pass(ifstream &input);
    bool fp_directive(string &line);
    bool fp_label(string &line);
    bool fp_instruction(string &line);
    bool fp_label_extended(string &line);
    
    bool second_pass(ifstream &input);
    bool sp_directive(string &line);
    bool sp_instruction(string &line);
    bool sp_label(string &line);
    bool sp_label_extended(string &line);

    bool is_end(string &line);
    bool edit_global_directives();
    int hexToInt(string to_convert);
    int match_index(smatch match);
    string get_opcode(string instr);
    string register_alias(string regg);
    string toLowerCase(string data);
    bool separate_symbols(vector<string> &tokens, string &to_split);

    InstructionInfo get_instr_info(string &line, int check_pass);


    void run_assembler();


private:
    string input_fstring;
    string output_fstring;
    SymbolTable* sym_table;
    SectionHeader* sec_header;
    Section curr_sec;

    list<SectionData*> all_sec_data_list;
    list<RelTable*> all_relt_list;
    vector<string> global_directives;
    map <string, string> instr_codes = 
    {
        {"halt", "00"}, 
        {"int", "10"}, 
        {"iret", "20"}, 
        {"call", "30"}, 
        {"ret", "40"},
        {"add", AR_OP "0"}, {"sub", AR_OP "1"}, {"mul", AR_OP "2"}, {"div", AR_OP "3"}, {"cmp", AR_OP "4"},
        {"not", LOG_OP "0"}, {"and", LOG_OP "1"},{"or", LOG_OP "2"}, {"xor", LOG_OP "3"}, {"test", LOG_OP "4"},
        {"shl", SH_OP "0"}, {"shr", SH_OP "1"},
        {"jmp", JMP_OP "0"}, {"jeq", JMP_OP "1"},  {"jne", JMP_OP "2"},  {"jgt", JMP_OP "3"},
        {"ldr", "A0"}, {"str", "B0"},
        {"xchg", "60"},
        {"push", "B0"},
        {"pop", "A0"}

    };
    string REG_DIR_TWOB_STR = RXP_START 
                            "(?:"
                            "(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr)\\s+" RXP_REG_MATCH "\\s*,\\s*" RXP_REG_MATCH "|"
                            "(int)\\s+" RXP_REG_MATCH "|"
                            "(not)\\s+" RXP_REG_MATCH
                            ")"
                            RXP_END;
    
    string PUSH_POP = RXP_START
                      "(?:"
                      "(push|pop)\\s+" RXP_REG_MATCH
                      ")"
                      RXP_END;
    
    string ONEB_STR = RXP_START "(halt|iret|ret)" RXP_END;
    string THREEB_JMP = RXP_START 
                        "(?:"
                        "(jmp|jeq|jne|jgt|call)\\s+\\*" RXP_REG_MATCH "|" 
                        "(jmp|jeq|jne|jgt|call)\\s+\\*" RXP_REG_IND
                        ")"
                        RXP_END;
    string THREEB_DATA = RXP_START
                         "(?:"
                         "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" RXP_REG_MATCH "|"
                         "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" RXP_REG_IND
                         ")"
                         RXP_END;

    string JMP_IMM = RXP_START
                     "(?:"
                     "(jmp|jeq|jne|jgt|call)\\s+" "(" RXP_ALF ")"   "|"         //jeq a  - ABS
                     "(jmp|jeq|jne|jgt|call)\\s+%" "(" RXP_ALF ")"  "|"        //jeq %a  -PCrel
                     "(jmp|jeq|jne|jgt|call)\\s+" "(" RXP_DIGIT ")" "|"       //jeq 55
                     "(jmp|jeq|jne|jgt|call)\\s+" RXP_HEX_MATCH             //jeq 0x55
                     ")"
                     RXP_END;

    string JMP_MEM = RXP_START
                     "(?:"
                     "(jmp|jeq|jne|jgt|call)\\s+\\*" "(" RXP_DIGIT ")"  "|"    //jeq *52     
                     "(jmp|jeq|jne|jgt|call)\\s+\\*" RXP_HEX_MATCH    "|"      //jeq *0x52     
                     "(jmp|jeq|jne|jgt|call)\\s+\\*" "(" RXP_ALF ")"          //jeq *mile    
                     ")"
                     RXP_END;


    string JMP_RID =  RXP_START
                       "(?:"
                       "(jmp|jeq|jne|jgt|call)\\s+\\*\\[\\s*" RXP_REG_MATCH "\\s*(\\+|-)\\s*" "(" RXP_ALF ")" "\\s*\\]" "|"       // jeq *[r7 + mystart]  21,22,23
                       "(jmp|jeq|jne|jgt|call)\\s+\\*\\[\\s*" RXP_REG_MATCH "\\s*(\\+|-)\\s*" "(" RXP_DIGIT ")" "\\s*\\]" "|"     //jeq *[r4 + 5] 15,16,17
                       "(jmp|jeq|jne|jgt|call)\\s+\\*\\[\\s*" RXP_REG_MATCH "\\s*(\\+|-)\\s*" RXP_HEX_MATCH "\\s*\\]"           //jeq *[r7 + 0x22] 18,19,20
                       ")"
                       RXP_END;


    string LDST_IMM = RXP_START
                     "(?:"
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" "\\$" "(" RXP_ALF ")" "|"    //ldr r0, $myStart
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" "\\$" "(" RXP_DIGIT ")" "|"  //ldr r0, $50
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" "\\$" RXP_HEX_MATCH          //ldr r0, $0x50
                     ")"
                     RXP_END;

    string LDST_MEM = RXP_START
                     "(?:"
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*"      "(" RXP_ALF ")" "|"     //ldr r0, myStart ABS
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" "%"  "(" RXP_ALF ")" "|"     //ldr r0, %myStart  PCREl
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*"      "(" RXP_DIGIT ")" "|"   //ldr r0, 10
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*"          RXP_HEX_MATCH       //ldr r0, 0x10
                     ")"
                     RXP_END;

    string LDST_RID = RXP_START
                     "(?:"
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" RXP_REG_IND_DISPL_A "|"      //ldr r0, [r4 + a]
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" RXP_REG_IND_DISPL_D "|"      //ldr r0, [r5 + 5]
                     "(ldr|str)\\s+" RXP_REG_MATCH "\\s*,\\s*" RXP_REG_IND_DISPL_H          //ldr r0, [r4 + 0x54]
                     ")"
                     RXP_END;
    
                        

};


#endif