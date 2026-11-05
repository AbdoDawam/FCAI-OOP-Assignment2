#include "machine.h"
#include <iostream>
#include <fstream>
using namespace std;

Machine::Machine() {
    resetMachine();
}

void Machine::displayMenu() {
    cout<<"Welcome to the Vole Machine!"<<endl;
}
bool Machine::loadProgramFile(string fileName,int startAddr) {
    //Get the file
    ifstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    // [added from AI to remove whitespaces]

    // std::string trim(const std::string& str) {
    //     size_t first = str.find_first_not_of(" \t\n\r\f\v");
    //     if (first == std::string::npos) return "";
    //     size_t last = str.find_last_not_of(" \t\n\r\f\v");
    //     return str.substr(first, (last - first + 1));
    // }

    //Make sure every line is cleared from spaces and is valid
    string line;
    int currentAddr = startAddr;
    while (getline(file, line)) {
        string clean;//=trim(line);
        if (clean.empty()||clean[0] == ';') {
            continue;
        }
        string instructions = line.substr(0,4);

        //put the HexNumber in a memory cell
        memory.setCell(currentAdress, instructions);
        //decrement the adder
        currentAddr++;
    }
    file.close();
    return true;
}


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
    cpu.reset();
}
int main() {
    Machine machine;
    machine.displayMenu();
}
