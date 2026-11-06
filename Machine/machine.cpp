#include "machine.h"
#include <iostream>
#include <fstream>
using namespace std;

Machine::Machine() {
    resetMachine();
}
void Machine::displayMenu() {
    int option = 0;
    while (option != 7) {
        cout<<"Welcome to the Vole Machine!"<<endl;
        cout << ":================================================:" << endl;
        cout << "1. Load File" << endl;
        cout << "2. Run All at Once" << endl;
        cout << "3. Run Step By Step " << endl;
        cout << "4. Overwrite" << endl;
        cout << "5. Show State" << endl;
        cout << "6. Reset" << endl;
        cout << "7. Exit" << endl;
        cin >> option;

        string fileName;
        int AddrNum;
        switch (option) {
            case 1: {
                cout << "Write File Name" << endl;
                cin >> fileName;
                cout << "Write Address Number you want to store the file in" << endl;
                cin >> AddrNum;
                bool State = loadProgramFile(fileName,AddrNum);
                if (State) {
                    cout<<"File loaded successfully!"<<endl;
                }
                else {
                    cout<<"Error: Could not open file."<<endl;
                };
                break;
            }

            case 2:
                cout << "Executing Program...";
                runAtOnce();
                break;

            case 3:
                cout << "Starting step-by-step mode." << endl;
                runStepByStep();
                break;

            case 4:
                overwriteValue();
                break;

            case 5:
                displayState();
                break;
            case 6:
                resetMachine();
                cout << "Machine has been reset to its default state." << endl;
                break;

            case 7:
                cout << "Exiting..." << endl;
                exit(0);                        //Ai

        }
        cout << ":================================================:" << endl;
    }
}
bool Machine::loadProgramFile(string fileName,int startAddr) {
    //Get the file
    ifstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    int currentAddr = startAddr;
    //Make sure every line is cleared from spaces and is valid
    string line;

    while (getline(file, line)) {
        string corrected_line;
        for (int i = 0; i < line.length(); i++) {
            if (line[i] == ' ') {
                continue;
            }
            else if (line[i] == ';') {
                break;
            }
            else {
                corrected_line.push_back(line[i]);
            }
        }
        //take the HexNumber from the line
        if (corrected_line.length() < 4) {
            continue;
        }
        string instructions = corrected_line.substr(0,4);

        //put the HexNumber in a memory cell
        memory.setCell(currentAddr, instructions);
        //decrement the adder
        currentAddr+=2;
    }
    file.close();
    return true;
}
void Machine::runAtOnce(int maxSteps) {
    for (int i = 0; i < maxSteps; i++) {
        cpu.fetch();                                //?
        cpu.decodeAndExecute();                     //?
        cpu.runNextStep();                          //?
        bool State = cpu.isHalted();
        if (State) {
            break;
        }
        displayState();

    }
}
void Machine::runStepByStep() {
    while (true) {
        bool state = cpu.isHalted();
        if (state) {
            cout<<"The program has already finished executing"<<endl;
            displayState();
            break;
        }
        else {
            cout<<"Press Enter for next step.";                             //?
            cin.ignore();
            cin.get();
            cpu.runNextStep();
            continue;
        }
    }
}
void Machine::overwriteValue() {
    int AddrNum = 0;
    int choice = 0;
    string HexNum;
    int registerNumber = 0;

    cout << "Which do you want to override ?";
    cout << "1. Memory" << endl;
    cout << "2. Register" << endl;
    cin >> choice;
    if (choice == 1) {
        cout << "Enter the Memory Address you want to override:" << endl;
        cin >> AddrNum;
        cout << "Enter new HexNumber:";
        cin >> HexNum;
        memory.setCell(AddrNum, HexNum);
    }
    else if (choice == 2) {
        cout << "Enter the Index of the Register you want to override:" << endl;
        cin >> registerNumber;
        cout << "Enter new HexNumber:";
        cin >> HexNum;
        cpu.setRegister(registerNumber,HexNum);
    }
}
void Machine::displayState() const{
    cout << "PC: " << cpu.getPC() << endl;                         //[all of it -> Ai]
    cout << "IR: " << cpu.getIR() << endl;

    // 2. Loop and print the 16 registers
    for (int i = 0; i < 16; i++) {
        cout << "Register " << i << ": " << cpu.getRegisters(i) << endl;
    }

    // 3. Loop and print the 256 memory cells
    cout << "Memory: " << endl;
    for (int j = 0; j < 256; j++) {
        cout << memory.getCell(j) << " ";
        if (j % 16 == 15) cout << endl; // Drops to a new line to make a grid
    }

    // 4. Print screen buffer
    cout << "Screen: " << screenBuffer << endl;
}
void Machine::resetMachine() {                            //[AI]
    cpu.reset();
    memory.reset();
    screenBuffer = "";
}

// =======================================================

int main() {
    Machine machine;
    machine.displayMenu();
}
