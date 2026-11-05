#ifndef MACHINE_H
#define MACHINE_H
#include <iostream>
using namespace std;

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
    const void displayState();
    void resetMachine();

};

#endif
