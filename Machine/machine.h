#ifndef MACHINE_H
#define MACHINE_H

#include <iostream>
#include <fstream>
#include <string>
#include "../Memory/Memory.h"
#include "../Storage/Storage.h"
#include "../CPU/CPU.h"

using namespace std;

class Machine {
    CPU cpu;
    Memory memory;
    string screenBuffer;
public:
    Machine();
    void displayMenu();
    bool loadProgramFile(string fileName, int startAddr = 0);
    void overwriteValue();
    void runAtOnce(int maxSteps = 1000);
    void runStepByStep();
    void displayState() const;
    void resetMachine();
};

#endif
