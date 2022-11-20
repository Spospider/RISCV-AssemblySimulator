#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

bool ExceptionFlag = 0; // terminate program

map<int, int> memory; // stack memory    Address|value in decimal

int r[32];  // registers

string instructions[40] = {"lui", "auipc", "jal", "jalr", "beq", "bne", "blt", 
                           "bge", "bltu", "bgeu", "lb", "lh", "lw", "lbu", "lhu", 
                           "sb", "sh", "sw", "addi", "slti", "sltiu", "xori", "ori", 
                           "andi", "slli", "srli", "srai", "add", "sub", "sll", "slt", 
                           "sltu", "xor", "srl", "sra", "or", "and", "fence", "ecall", "ebreak"};  //...  risc v instructions

map<string, int> reg; // register names to register number



map<string, pair<int, int>> Labels;  // contains label, and its calling, and label position,   <L , <A, B>>   offset = B - A  | A =  calling location, B = label location
struct inst {
    string type;
    int rs0 = -1;
    int rs1 = -1;
    int rs2 = -1;
    int immediate = 0;
    string label;
};
vector<inst> program;  // the instructions data after they get read, 
                            //they get executed one by one, program counter is choosing which one to run;
int PC = 0;  // program counter pointing to first instruction

// Functions

void ADD( inst x) //1
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0] = r[x.rs1]+r[x.rs2];
}
    

void SUB( inst x) //2
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0] = r [x.rs1]-r [x.rs2];
    
}
    void XOR(inst x) //4
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0]=r [x.rs1] ^ r [x.rs2];

}
void OR(inst x) //5
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0]=r [x.rs1] | r [x.rs2];

}
void AND(inst x) //6
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0]=r [x.rs1] & r [x.rs2];

}
void SLL(inst x) //7 
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0] =r [x.rs1] << r [x.rs2];

}
void SLT(inst x) //8
{
    if (x.rs0 == 0)
        return ;
    if (r [x.rs1] < r[x.rs2]){
        r[x.rs0] = 1;
    }
    else
        r[x.rs0] = 0;

}
void SLTU(inst x) //9
{
    if (x.rs0 == 0)
        return ;
    if (r[x.rs1] < r[x.rs2])
        x.rs0  = 1;
    else
        x.rs0  = 0 ;

}
void SRL(inst x) //10
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0] = r[x.rs1] >> r[x.rs2] ;

}
void SRA(inst x) //11
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0] = r[x.rs1] >> r[x.rs2];

}
void ADDI(inst x) //12
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0]=r[x.rs1] + x.immediate;

}
void ORI(inst x) //13
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0]=r[x.rs1] || x.immediate;

}
void XORI(inst x) //14
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0]=r[x.rs1] ^ x.immediate;

}
void ANDI(inst x) //15
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0]=r[x.rs1] && x.immediate;

}
void SLLI(inst x) //16
{
    if (x.rs0 == 0)
        return ;
    r[x.rs0] = r[x.rs1] << r[x.rs2];
}
void SLTI(inst x) //17
{
    if (r[x.rs1]< x.immediate)
        r[x.rs0]=1;
    else r[x.rs0]=0;
}
void SLTIU(inst x) //18
{
    if ((unsigned int32_t)r[x.rs1]< x.immediate)
        r[x.rs0]=1;
    else r[x.rs0]=0;  
}
void SRLI(inst x) //19
{
    r[x.rs0] = (unsigned int)r[x.rs1] >> (unsigned int)r[x.immediate];
}
void SRAI(inst x) //20
{
    r[x.rs0] = r[x.rs1] >> r[x.immediate];

}
void LB(inst x) //21
{
    int add = x.immediate/4 + r[x.rs2];
    if (memory.find(add) != memory.end() )
    {
        int temp = memory[add];
        temp = temp<<24;
        temp = temp>>24;
        r[x.rs0] = temp;
    }
    else
    {
        r[x.rs0] = 0;
    }
    return;
}
void LBU(inst x) //22
{
    int add = x.immediate/4 + r[x.rs2];
    if ( memory.find(add) != memory.end() )
    {
        int temp = memory[add];
        temp = (unsigned int)temp<<24;
        temp = (unsigned int)temp>>24;
        r[x.rs0] = (unsigned int)temp;
    }
    else
    {
        r[x.rs0] = 0;
    }
    return;
}
void LH(inst x) //23
{
    int add = x.immediate/4 + r[x.rs2];
    if ( memory.find(add) != memory.end() )
    {
        int temp = memory[add];
        int t1 = memory[add+1];
        temp = t1;
        temp =  temp <<= 8;
        t1 = memory[add];
        temp |= t1;
        temp = temp<<16;
        temp = temp>>16;
        r[x.rs0] = temp;
    }
    else
    {
        r[x.rs0] = 0;
    }
    return;
}
void LHU(inst x) //24
{
    int add = x.immediate/4 + r[x.rs2];
    if ( memory.find(add) != memory.end() )
    {
        int temp = memory[add];
        int t1 = memory[add+1];
        temp = t1;
        temp =  temp <<= 8;
        t1 = memory[add];
        temp |= t1;
        temp = (unsigned int)temp<<16;
        temp = (unsigned int)temp>>16;
        r[x.rs0] = (unsigned int)temp;
    }
    else
    {
        r[x.rs0] = 0;
    }
    return;
}
void LW(inst x) //25
{
    int add = x.immediate/4 + r[x.rs1];
    if ( memory.find(add) != memory.end() )
    {
        int temp = memory[add+3];
        temp =  temp <<= 8;
        int t1 = memory[add+2];
        temp |= t1;
        temp =  temp <<= 8;
        t1 = memory[add+1];
        temp |= t1;
        temp =  temp <<= 8;
        t1 = memory[add];
        temp |= t1;
        r[x.rs0] = temp;
    }
    else
    {
        r[x.rs0] = 0;
    }
    return;
}

void SB(inst x) //26
{
    int add = x.immediate/4 + r[x.rs2];
    int temp = r[x.rs1];
    temp = temp<<24;
    temp = temp>>24;
    memory[add] = temp;
    return;  

}
void SH(inst x) //27
{
    int add = x.immediate/4 + r[x.rs2];
    int temp = r[x.rs1];
    temp = temp<<24;
    temp = temp>>24;
    memory[add] = temp;
    temp = r[x.rs1]>>8;
    temp = r[x.rs1]<<24;
    memory[add+1] = temp;
    return;    
}
void SW(inst x) //28
{
    int add = x.immediate/4 + r[x.rs2];
    int temp = r[x.rs1];
    temp = temp<<24;
    temp = temp>>24;
    memory[add] = temp;
    temp = r[x.rs1]>>8;
    temp = r[x.rs1]<<24;
    memory[add+1] = temp;
    temp = r[x.rs1]>>16;
    temp = r[x.rs1]<<24;
    memory[add+2] = temp;
    temp = r[x.rs1]>>24;
    temp = r[x.rs1]<<24;
    memory[add+3] = temp;
    return;        
}
void BEQ(inst x) //29
{
    if (r[x.rs0]==r[x.rs1])
        PC += Labels[x.label].second - Labels[x.label].first - 1;        

}
void BNE(inst x) //30
{
    if (r[x.rs0] != r[x.rs1])
        PC += Labels[x.label].second - Labels[x.label].first - 1;
           
}
void BLT(inst x) //31
{
    if (r[x.rs0] < r[x.rs1])
        PC += Labels[x.label].second - Labels[x.label].first - 1;;

}
void BGE(inst x) //32
{
    if (r[x.rs0] >= r[x.rs1])
        PC += Labels[x.label].second - Labels[x.label].first - 1;;
}
void BLTU(inst x) //33
{
    if ((unsigned int)r[x.rs0] < (unsigned int)r[x.rs1])
        PC += Labels[x.label].second - Labels[x.label].first - 1;;
    
}
void BGEU(inst x) //34
{
    if ((unsigned int)r[x.rs0] >= (unsigned int)r[x.rs1])
        PC += Labels[x.label].second - Labels[x.label].first - 1;;
}

void LUI(inst x) //35
{
    r[x.rs0]=0;
    r[x.rs0]=x.immediate<<12;
    PC += Labels[x.label].second - Labels[x.label].first - 1;;
}
void AUIPC(inst x) //36
{
    r[x.rs0] = PC + x.immediate << 12;
    PC += Labels[x.label].second - Labels[x.label].first - 1;; 
}

void JAL(inst x) //37
{
    PC += Labels[x.label].second - Labels[x.label].first - 1;;
    r[x.rs0] = PC+1;

}
void JALR(inst x) //38
{
    r[x.rs0] = PC + 1;
    PC = r[x.rs1]+(x.immediate/4);
}

void ECALL(inst x) //39
{
    exit(0);
}
void FENCE(inst x) //40
{
    exit(0);
}
void EBREAK(inst x) //41
{
    exit(0);
}

// End Function

void initialize() {  //  for intitalizing anyy arrays, maps etc.
    reg["zero"] = 0;
    reg["ra"] = 1;
    reg["sp"] = 2;
    reg["gp"] = 3;
    reg["tp"] = 4;
    reg["t0"] = 5;
    reg["t1"] = 6;
    reg["t2"] = 7;
    reg["s0"] = 8;
    reg["fp"] = 8;
    reg["s1"] = 9;
    r[0] = 0;
    for (int i =0;i <= 7 ; i++) {
        reg["a" + to_string(i)] = 10+i;
    }
    for (int i =2;i <= 11 ; i++) {
        reg["s" + to_string(i)] = 16+i;
    }
    for (int i =0;i <= 3 ; i++) {
        reg["t" + to_string(3+i)] = 28+i;
    }
}

int assignReg(string name) {
    if (tolower(name[0]) == 'x') { // if reg is x0 ,x1, etc
        string t = name;
        t.erase(0, 1);
        return stoi(t);
    }
    else {
        if (reg.find(name) == reg.end()) {
            return -1; 
        }
        return reg[name];
    }
}

void ReadSource() {
    cout << "Please input Assembly program file name" << endl;
    string n;
    cin >> n;
    ifstream codeF(n);
    
    string line;
    int lineNum = 0;
    while(getline(codeF, line)) {
        vector<string> mystring;
        istringstream ss(line);
        string token;

        inst command;
        //Separate string based on commas and white spaces
        getline(ss,token,' ');
        if (token.length() == line.length()) {
            if (Labels.find(token) == Labels.end()) {  // label not yet defined
                pair<int, int> t;
                t.first = -1;
                t.second = lineNum;
                Labels[token] = t;
            }
            else {
                Labels[token].second = lineNum;
            }
            //cout << "here" << token << " ";
            getline(codeF, line);
            
            ss.clear();
            ss.str(line);
            
            getline(ss,token,' ');
            cout << "here" << token << " ";
        }
        transform(token.begin(), token.end(), token.begin(), ::tolower);
        while (token.length() == 0) {
            getline(ss,token,' ');
        }
        if (token.find("//") < token.length()) { // commented line
            continue;
        }
        if (token.find(':') == token.length() - 1) {
            //label found
            if (Labels.find(token) == Labels.end()) {  // label not yet defined
                pair<int, int> t;
                t.first = -1;
                t.second = lineNum;
                Labels[token] = t;
            }
            else {
                Labels[token].second = lineNum;
            }
            
            getline(ss, token, ' ');
            transform(token.begin(), token.end(), token.begin(), ::tolower);
            command.type = token;
        }
        else if (token.find(':') >= token.length()) {
            // add inst name
            transform(token.begin(), token.end(), token.begin(), ::tolower);
            command.type = token;
        }
        else {
            //syntax error
            cout << "syntax error at line " << lineNum << endl;
            ExceptionFlag = true;
            return;
        }
        //cout << command.type << "  ";
        
        vector<string> temps;
        while (getline(ss, token,',')){
            token.erase(std::remove(token.begin(), token.end(), ' '), token.end());
            if (token.find('(') < token.length()) {  // for e.g. 0(x5) cases
                istringstream temp(token);
                string t1;
                getline(temp, t1, '(');
                temps.push_back(t1);
                getline(temp, t1, '(');
                t1.erase(std::remove(t1.begin(), t1.end(), ')'), t1.end());
                token = t1;
            }
            temps.push_back(token);
            //cout << token << "  ";
            //i++;
        }
        for (int i = 0; i < temps.size(); i++) {
            if((int(temps[i][0]) >= 48 && int(temps[i][0]) <= 57) || int(temps[i][0]) == 45) {  // if its a number // for normal immediates
                command.immediate = stoi(temps[i]);
            }
            else {
                if (command.rs0 == -1)
                    command.rs0 = assignReg(temps[i]);
                else if (command.rs1 == -1)
                    command.rs1 = assignReg(temps[i]);
                else if (command.rs2 == -1)
                    command.rs2 = assignReg(temps[i]);
                if (assignReg(temps[i]) == -1 && i == temps.size() - 1) {
                    command.label = temps[i];
                    if (Labels.find(temps[i]) == Labels.end()) {  // label not yet defined
                        pair<int, int> t;
                        t.first = lineNum;
                        Labels[temps[i]] = t;
                    }
                    else {
                        Labels[temps[i]].first = lineNum;
                    }
                }
            }
        }
        
        program.push_back(command);
        lineNum++;
    }

    /*// DEBUG
    cout << "READ:" << endl;
    for (int i =0; i < program.size() ; i++) {
        cout << program[i].type << " rs0= " << program[i].rs0 << " rs1= " << program[i].rs1 << " rs2= " << program[i].rs2 << " imm= " << program[i].immediate  << " label= " << program[i].label << endl;
    }
    cout << "lines: "  << lineNum << endl;
    // END DEBUG*/
    codeF.close();
    
    cout << "Please input program data file name(type x if none)" << endl;
    cin >> n;
    if (n.length() == 0) {
        return;
    }
    codeF.open(n);
    while(getline(codeF, line)) {  // syntax m123 : 123   or r3 : 23
        remove(line.begin(), line.end(), ' ');
        istringstream ss(line);
        string t;
        getline(ss, t, ':');
        if (t[0] == 'm') {
            t.erase(0, 1);
            int addr = stoi(t);
            getline(ss, t, ':');
            int val = stoi(t);
            memory[addr] = val;
            cout << "m" << addr << " = " << val << endl;
        }
        else if (t[0] == 'r') {
            t.erase(0, 1);
            int reg = stoi(t);
            getline(ss, t, ':');
            int val = stoi(t);
            if(reg > 0 && reg < 32) { 
                r[reg] = val;
                cout << "x" << reg << " = " << val << endl;
            }
        }
    }

    codeF.close();

}

void callFunc(inst x) {
    string name = x.type;
    if(name == "lui") {
    LUI(x);
    }
    if(name == "auipc") {
    AUIPC(x);
    }
    if(name == "jal") {
    JAL(x);
    }
    if(name == "jalr") {
    JALR(x);
    }
    if(name == "beq") {
    BEQ(x);
    }
    if(name == "bne") {
    BNE(x);
    }
    if(name == "blt") {
    BLT(x);
    }
    if(name == "bge") {
    BGE(x);
    }
    if(name == "bltu") {
    BLTU(x);
    }
    if(name == "bgeu") {
    BGEU(x);
    }
    if(name == "lb") {
    LB(x);
    }
    if(name == "lh") {
    LH(x);
    }
    if(name == "lw") {
    LW(x);
    }
    if(name == "lbu") {
    LBU(x);
    }
    if(name == "lhu") {
    LHU(x);
    }
    if(name == "sb") {
    SB(x);
    }
    if(name == "sh") {
    SH(x);
    }
    if(name == "sw") {
    SW(x);
    }
    if(name == "addi") {
    ADDI(x);
    }
    if(name == "slti") {
    SLTI(x);
    }
    if(name == "sltiu") {
    SLTIU(x);
    }
    if(name == "xori") {
    XORI(x);
    }
    if(name == "ori") {
    ORI(x);
    }
    if(name == "andi") {
    ANDI(x);
    }
    if(name == "slli") {
    SLLI(x);
    }
    if(name == "srli") {
    SRLI(x);
    }
    if(name == "srai") {
    SRAI(x);
    }
    if(name == "add") {
    ADD(x);
    }
    if(name == "sub") {
    SUB(x);
    }
    if(name == "sll") {
    SLL(x);
    }
    if(name == "slt") {
    SLT(x);
    }
    if(name == "sltu") {
    SLTU(x);
    }
    if(name == "xor") {
    XOR(x);
    }
    if(name == "srl") {
    SRL(x);
    }
    if(name == "sra") {
    SRA(x);
    }
    if(name == "or") {
    OR(x);
    }
    if(name == "and") {
    AND(x);
    }
    if(name == "fence") {
    //FENCE(x);
    cout << "end of program" << endl;
    ExceptionFlag = true;
    }
    if(name == "ecall") {
    //ECALL(x);
    cout << "end of program" << endl;
    ExceptionFlag = true;
    }
    if(name == "ebreak") {
    //EBREAK(x);
    cout << "end of program" << endl;
    ExceptionFlag = true;
    }
    PC++;
}

void Showdata() {
    //system("cls");
    cout << "------------------------" << endl;
    cout << "Registers" << '\t' << "Dec" << '\t' << "Hex"  << '\t' << "Bin" << endl;
    for (int i = 0 ; i < 32; i++) { // print the registers
        char hexString[7];
        char bin[7];
        itoa(r[i], hexString, 16);
        itoa(r[i], bin, 2);
        cout << 'x' << i << '\t' << '\t' << r[i] << '\t' << hexString << '\t' << bin << endl;
    }
    cout << "Mem Address" << '\t' << "Dec" << '\t' << "Hex"  << '\t' << "Bin" << endl;
    for(std::map<int,int>::iterator iter = memory.begin(); iter != memory.end(); ++iter)
    {
        char hexString[7];
        char bin[7];
        itoa(iter->second, hexString, 16);
        itoa(iter->second, bin, 2);
        cout << "mem[" << iter->first << ']' << '\t' << '\t' << iter->second << '\t' << hexString << '\t' << bin << endl;
        //Key k = iter->first;   memory address
        //Value v = iter->second;
    }
}

int main() {
    cout <<"Welcome to the Assembly simulator!" << endl;
    initialize();  //  get program ready
    ReadSource();

    cout << "Please enter the program's starting address" << endl;
    int addressStart;
    cin >> addressStart;
    
    char r;
    bool repeat = false;
    do {
        cout << "Please select the mode you'd like to use" << endl << "1: one by one instruction" <<  endl << "2: run full program" << endl;
        cin >> r;
        int cycle = 0;
        if (r == '2') {
            repeat = false;
            while (PC < program.size() && ExceptionFlag == false) {
                cout << "cycle " << cycle << '\t' << "PC = " << PC + addressStart;
                callFunc(program[PC]);
                Showdata();
                cycle++;
            }
            r = '1';
        }
        else if (r == '1') {
            while (PC < program.size() && ExceptionFlag == false) {
                cout << "Enter any key to continue" << endl;
                cin >> r;
                cout << "cycle " << cycle << '\t' << "PC = " << PC + addressStart;
                callFunc(program[PC]);
                Showdata();
                cycle++;
            }
            r = '1';
        }
    }while(r != '1' && r != '2');
    cout << "--------------------------END OF PROGRAM--------------------------" << endl;
}
