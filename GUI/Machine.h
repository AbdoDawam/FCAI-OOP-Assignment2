#ifndef MACHINE_H
#define MACHINE_H

#include "CPU.h"
#include "Memory.h"
#include <string>

class Machine {
private:
    CPU cpu;
    Memory memory;
    std::string screenBuffer;

public:
    Machine();

    void displayMenu();
    bool loadProgramFile(std::string filename, int startAddr = 0);
    void overwriteValue();
    void runAtOnce(int maxSteps = 1000);
    void runStepByStep();
    void displayState() const;
    void resetMachine();

    bool loadProgramFromText(const std::string& sourceText, int startAddr,
                             std::string& outMessage, int& outInstructionCount);
    bool loadProgramFileWithReport(const std::string& filename, int startAddr,
                                   std::string& outMessage, int& outInstructionCount,
                                   std::string& outSourceText);
    bool executeSingleStep();
    int executeAllAtOnce(int maxSteps = 1000);
    void haltExecution();

    const CPU& getCPU() const;
    CPU& getCPU();
    const Memory& getMemory() const;
    Memory& getMemory();
    const std::string& getScreenBuffer() const;
};

#endif
