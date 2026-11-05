#include "machine.h"
#include <iostream>
using namespace std;

Machine::Machine(){}

void Machine::displayMenu() {
    cout<<"Welcome to the Vole Machine!"<<endl;
}
bool Machine::loadProgramFile(string fileName,int startAddr){}
void Machine::overwriteValue() {
    //setCell()
}
void Machine::runAtOnce(int maxSteps) {
    //CPU()
}
void Machine::runStepByStep() {
    //??
}
const void Machine::displayState(){}

void Machine::resetMachine() {
    ~Machine();
}

int main() {
    Machine machine;
    machine.displayMenu();
}
