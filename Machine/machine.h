#ifndef MACHINE_H
#define MACHINE_H
#include <iostream>
#include <fstream>
using namespace std;

#include "CPU.h"
#include "Memory.h"

class Machine{
    CPU cpu;
    Memory memory;
    string screenBuffer;
public:
    Machine();
    void displayMenu();
    bool loadProgramFile(string fileName,int startAddr = 0);
    void overwriteValue();
    void runAtOnce(int maxSteps= 1000);
    void runStepByStep();
    void displayState () const;
    void resetMachine();

};

#endif
