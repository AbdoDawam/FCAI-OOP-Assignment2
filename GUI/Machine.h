/*
 * File: Machine.h
 * Date: 6 Oct 1973
 * Package: Orchestration
 * Description: Top-level orchestrator for the Vole Machine Simulator.
 */

#ifndef MACHINE_H
#define MACHINE_H

#include "CPU.h"
#include "Memory.h"
#include <string>
#include <vector>

class Machine {
private:
    CPU cpu;
    Memory memory;
    std::string screenBuffer;
    std::vector<std::string> executionLog;

public:
    Machine();

    // Standard UML Methods
    void displayMenu();
    bool loadProgramFile(std::string filename, int startAddr = 0);
    void overwriteValue();
    void runAtOnce(int maxSteps = 1000);
    void runStepByStep();
    void displayState() const;
    void resetMachine();

    // GUI Integration & Extended Control Methods
    bool loadProgramFromText(const std::string& sourceText, int startAddr,
                             std::string& outMessage, int& outInstructionCount);
    bool loadProgramFileWithReport(const std::string& filename, int startAddr,
                                   std::string& outMessage, int& outInstructionCount,
                                   std::string& outSourceText);
    bool executeSingleStep();
    int executeAllAtOnce(int maxSteps = 1000);
    void haltExecution();
    void clearMemoryOnly();

    const CPU& getCPU() const;
    CPU& getCPU();
    const Memory& getMemory() const;
    Memory& getMemory();
    const std::string& getScreenBuffer() const;
    void clearScreenBuffer();
    const std::vector<std::string>& getExecutionLog() const;
    void addLogEntry(const std::string& entry);
};

#endif // MACHINE_H
