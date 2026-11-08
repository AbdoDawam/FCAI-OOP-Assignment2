#ifndef CPU_H
#define CPU_H

#include "ALU.h"
#include "Memory.h"
#include "Register.h"
#include <string>

class CPU {
private:
    int pc;
    std::string ir;
    bool halted;
    Register reg;
    ALU alu;

    int lastExecutedPC;
    int lastModifiedReg;
    int lastWrittenMem;
    std::string lastActionDescription;

public:
    CPU();

    void fetch(const Memory& mem);
    void decodeAndExecute(Memory& mem, std::string& screenBuffer);
    bool runNextStep(Memory& mem, std::string& screenBuffer);
    static bool isValidInstruction(std::string inst);

    void setPC(int addr);
    int getPC() const;
    std::string getIR() const;
    bool isHalted() const;
    void halt();
    const Register& getRegisters() const;
    void setRegister(int index, std::string val);
    void reset(int startAddr = 0);

    int getLastModifiedReg() const;
    int getLastWrittenMem() const;
    std::string getLastActionDescription() const;
};

#endif
