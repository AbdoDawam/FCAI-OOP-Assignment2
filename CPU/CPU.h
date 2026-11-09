#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <string>
#include "../Register/Register.h"
#include "../Memory/Memory.h"
#include "../ALU/ALU.h"

using namespace std;

class CPU {
private:
    int pc;
    string ir;
    bool halted;
    Register reg;
    ALU alu;
public:
    CPU();
    void fetch(const Memory& mem);
    void decodeAndExecute(Memory& mem, string& screenBuffer);
    bool runNextStep(Memory& mem, string& screenBuffer);
    bool isValidInstruction(string inst);
    void setPC(int addr);
    int getPC() const;
    string getIR() const;
    bool isHalted() const;
    const Register& getRegister() const;
    void setRegister(int index, string val);
    void reset(int startAddress = 0);
};

#endif
