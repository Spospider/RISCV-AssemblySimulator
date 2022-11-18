#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

bool ExceptionFlag = 0; // terminate program

map<int, int> memory; // stack memory    Address|value in decimal

int r[32];  // registers

string instructions[40] = {"add", "addi"};  //...  risc v instructions

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
    for (int i =0;i <= 7 ; i++) {
        reg["a" + to_string(i)] = 10+i;
    }
    for (int i =2;i <= 11 ; i++) {
        reg["s" + to_string(i)] = 16+i;
    }
    for (int i =0;i <= 3 ; i++) {
        reg["t" + to_string(3+i)] = 28+i;
    }
    //...
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
        transform(token.begin(), token.end(), token.begin(), ::tolower);
        if (token.find("//") ==  0) { // commended line
            continue;
        }
        if (token.find(':') == token.length() - 1) {
            //label found
            pair<int, int> t;
            t.first = -1;
            t.second = lineNum;
            Labels[token] = t;
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

    // DEBUG
    cout << "READ:" << endl;
    for (int i =0; i < program.size() ; i++) {
        cout << program[i].type << " rs0= " << program[i].rs0 << " rs1= " << program[i].rs1 << " rs2= " << program[i].rs2 << " imm= " << program[i].immediate  << " label= " << program[i].label << endl;
    }
    // END DEBUG
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

int main() {
    initialize();  //  get program ready
    ReadSource();
    cout <<"done";
}
