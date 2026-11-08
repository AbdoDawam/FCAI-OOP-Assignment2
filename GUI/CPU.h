/*
 * File: CPU.h
 * Date: 6 Oct 1973
 * Package: Processing
 * Description: Central Processing Unit for the Vole Machine Simulator.
 */

#ifndef CPU_H
#define CPU_H

#include "ALU.h"
#include "Memory.h"
#include "Register.h"
#include <string>
#include <vector>

class CPU {
private:
    int pc;
    std::string ir;
    bool halted;
    Register reg;
    ALU alu;

    // Diagnostic tracking for GUI visualization
    int lastExecutedPC;
    int lastModifiedReg;
    int lastWrittenMem;
    int stepCount;
    std::string lastActionDescription;
    std::vector<std::string> screenHexHistory;

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

    // GUI & Diagnostic Helpers
    const ALU& getALU() const;
    int getLastExecutedPC() const;
    int getLastModifiedReg() const;
    int getLastWrittenMem() const;
    int getStepCount() const;
    std::string getLastActionDescription() const;
    const std::vector<std::string>& getScreenHexHistory() const;
    static std::string disassembleInstruction(const std::string& inst);
};

#endif // CPU_H
