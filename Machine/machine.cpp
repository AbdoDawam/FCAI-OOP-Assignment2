#include "machine.h"
#include <iostream>
#include <fstream>
#include <cctype>
#include <limits>

using namespace std;

Machine::Machine() {
    resetMachine();
}

void Machine::displayMenu() {
    int option = 0;
    while (option != 7) {
        cout << "Welcome to the Vole Machine!" << endl;
        cout << ":================================================:" << endl;
        cout << "1. Load File" << endl;
        cout << "2. Run All at Once" << endl;
        cout << "3. Run Step By Step " << endl;
        cout << "4. Overwrite" << endl;
        cout << "5. Show State" << endl;
        cout << "6. Reset" << endl;
        cout << "7. Exit" << endl;
        if (!(cin >> option)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        string fileName;
        int AddrNum = 0;
        switch (option) {
            case 1: {
                cout << "Write File Name" << endl;
                getline(cin >> ws, fileName);
                cout << "Write Address Number you want to store the file in" << endl;
                cin >> AddrNum;
                bool State = loadProgramFile(fileName, AddrNum);
                if (State) {
                    cout << "File loaded successfully!" << endl;
                } else {
                    cout << "Error: Could not open file or invalid start address." << endl;
                }
                break;
            }

            case 2:
                cout << "Executing Program..." << endl;
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
                return;
        }
        cout << ":================================================:" << endl;
    }
}

bool Machine::loadProgramFile(string fileName, int startAddr) {
    if (startAddr < 0 || startAddr >= 255 || startAddr % 2 != 0) {
        return false;
    }

    ifstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    int currentAddr = startAddr;
    string line;

    while (getline(file, line)) {
        string corrected_line;
        for (size_t i = 0; i < line.length(); i++) {
            if (isspace(static_cast<unsigned char>(line[i]))) {
                continue;
            } else if (line[i] == ';') {
                break;
            } else {
                corrected_line.push_back(line[i]);
            }
        }

        if (corrected_line.length() < 4) {
            continue;
        }

        string instructions = corrected_line.substr(0, 4);
        if (!ALU::isValidHex(instructions)) {
            continue;
        }

        if (currentAddr + 1 >= memory.getSize()) {
            break;
        }

        memory.setCell(currentAddr, instructions.substr(0, 2));
        memory.setCell(currentAddr + 1, instructions.substr(2, 2));
        currentAddr += 2;
    }
    file.close();
    cpu.setPC(startAddr);
    return true;
}

void Machine::runAtOnce(int maxSteps) {
    for (int i = 0; i < maxSteps; i++) {
        if (cpu.isHalted()) {
            break;
        }
        cpu.runNextStep(memory, screenBuffer);
    }
    displayState();
}

void Machine::runStepByStep() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    while (true) {
        if (cpu.isHalted()) {
            cout << "The program has finished executing." << endl;
            displayState();
            break;
        } else {
            cout << "Press Enter for next step...";
            cin.get();
            cpu.runNextStep(memory, screenBuffer);
            displayState();
        }
    }
}

void Machine::overwriteValue() {
    int AddrNum = 0;
    int choice = 0;
    string HexNum;
    int registerNumber = 0;

    cout << "Which do you want to override ?" << endl;
    cout << "1. Memory" << endl;
    cout << "2. Register" << endl;
    cin >> choice;
    if (choice == 1) {
        cout << "Enter the Memory Address you want to override:" << endl;
        cin >> AddrNum;
        cout << "Enter new HexNumber:";
        cin >> HexNum;
        memory.setCell(AddrNum, HexNum);
    } else if (choice == 2) {
        cout << "Enter the Index of the Register you want to override:" << endl;
        cin >> registerNumber;
        cout << "Enter new HexNumber:";
        cin >> HexNum;
        cpu.setRegister(registerNumber, HexNum);
    }
}

void Machine::displayState() const {
    cout << "PC: " << cpu.getPC() << endl;
    cout << "IR: " << cpu.getIR() << endl;

    // 2. Loop and print the 16 registers
    for (int i = 0; i < 16; i++) {
        cout << "Register " << i << ": " << cpu.getRegister().getReg(i) << endl;
    }

    // 3. Loop and print the 256 memory cells
    cout << "Memory: " << endl;
    for (int j = 0; j < 256; j++) {
        cout << memory.getCell(j) << " ";
        if (j % 16 == 15) cout << endl;
    }

    // 4. Print screen buffer
    cout << "Screen: " << screenBuffer << endl;
}

void Machine::resetMachine() {
    cpu.reset();
    memory.clear();
    screenBuffer = "";
}
