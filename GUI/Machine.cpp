#include "Machine.h"
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Machine::Machine() {
    resetMachine();
}

bool Machine::loadProgramFromText(const std::string& sourceText, int startAddr,
                                  std::string& outMessage, int& outInstructionCount) {
    outInstructionCount = 0;

    if (startAddr < 0 || startAddr >= 255 || (startAddr % 2 != 0)) {
        outMessage = "Start address must be an even address between 00 and FE.";
        return false;
    }

    std::istringstream stream(sourceText);
    std::string line;
    std::vector<std::string> instructions;
    int lineNum = 0;

    while (std::getline(stream, line)) {
        lineNum++;
        std::size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        std::string cleaned;
        for (char c : line) {
            if (!std::isspace(static_cast<unsigned char>(c))) {
                cleaned.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
            }
        }

        if (cleaned.empty()) {
            continue;
        }

        if (cleaned.length() < 4) {
            outMessage = "Line " + std::to_string(lineNum) + ": Invalid instruction length.";
            return false;
        }

        std::string inst = cleaned.substr(0, 4);
        if (!CPU::isValidInstruction(inst)) {
            outMessage = "Line " + std::to_string(lineNum) + ": Invalid instruction '" + inst + "'.";
            return false;
        }

        instructions.push_back(inst);
    }

    if (instructions.empty()) {
        outMessage = "No instructions found.";
        return false;
    }

    if (startAddr + static_cast<int>(instructions.size()) * 2 > 256) {
        outMessage = "Program exceeds memory size.";
        return false;
    }

    int currentAddr = startAddr;
    for (const std::string& inst : instructions) {
        memory.setCell(currentAddr, inst.substr(0, 2));
        memory.setCell(currentAddr + 1, inst.substr(2, 2));
        currentAddr += 2;
    }

    cpu.reset(startAddr);
    outInstructionCount = static_cast<int>(instructions.size());
    outMessage = "Loaded " + std::to_string(outInstructionCount) +
                 " instructions at " + ALU::decToHex(startAddr, 2) + ".";
    return true;
}

bool Machine::loadProgramFileWithReport(const std::string& filename, int startAddr,
                                        std::string& outMessage, int& outInstructionCount,
                                        std::string& outSourceText) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        outMessage = "Could not open file.";
        outInstructionCount = 0;
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    outSourceText = buffer.str();
    file.close();

    return loadProgramFromText(outSourceText, startAddr, outMessage, outInstructionCount);
}

bool Machine::loadProgramFile(std::string filename, int startAddr) {
    std::string msg;
    std::string src;
    int count = 0;
    return loadProgramFileWithReport(filename, startAddr, msg, count, src);
}

bool Machine::executeSingleStep() {
    if (cpu.isHalted()) return false;
    return cpu.runNextStep(memory, screenBuffer);
}

int Machine::executeAllAtOnce(int maxSteps) {
    int executed = 0;
    while (!cpu.isHalted() && executed < maxSteps) {
        executeSingleStep();
        executed++;
    }
    return executed;
}

void Machine::runAtOnce(int maxSteps) {
    executeAllAtOnce(maxSteps);
    displayState();
}

void Machine::runStepByStep() {
    while (!cpu.isHalted()) {
        executeSingleStep();
        displayState();
        if (cpu.isHalted()) break;
        std::cin.ignore(10000, '\n');
    }
}

void Machine::overwriteValue() {
    int choice = 0;
    std::cout << "1. Memory\n2. Register\nChoice: ";
    std::cin >> choice;
    if (choice == 1) {
        std::string addrHex, valHex;
        std::cout << "Address (00-FF): ";
        std::cin >> addrHex;
        std::cout << "Value (00-FF): ";
        std::cin >> valHex;
        if (ALU::isValidHex(addrHex) && ALU::isValidHex(valHex)) {
            memory.setCell(ALU::hexToDec(addrHex), valHex);
        }
    } else if (choice == 2) {
        std::string regHex, valHex;
        std::cout << "Register (0-F): ";
        std::cin >> regHex;
        std::cout << "Value (00-FF): ";
        std::cin >> valHex;
        if (ALU::isValidHex(regHex) && ALU::isValidHex(valHex)) {
            cpu.setRegister(ALU::hexToDec(regHex), valHex);
        }
    }
}

void Machine::displayState() const {
    std::cout << "PC: " << ALU::decToHex(cpu.getPC(), 2)
              << " | IR: " << cpu.getIR() << "\n";
    const Register& regs = cpu.getRegisters();
    for (int i = 0; i < 16; ++i) {
        std::cout << "R" << ALU::decToHex(i, 1) << ": " << regs.getReg(i)
                  << ((i % 4 == 3) ? "\n" : "  ");
    }
    for (int r = 0; r < 16; ++r) {
        for (int c = 0; c < 16; ++c) {
            std::cout << memory.getCell(r * 16 + c) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "Screen: " << screenBuffer << "\n";
}

void Machine::displayMenu() {
    int option = 0;
    while (option != 7) {
        std::cout << "1. Load File\n2. Run At Once\n3. Run Step By Step\n"
                  << "4. Overwrite\n5. Show State\n6. Reset\n7. Exit\nOption: ";
        if (!(std::cin >> option)) break;
        switch (option) {
            case 1: {
                std::string fname, startHex;
                std::cin >> fname >> startHex;
                int startAddr = ALU::isValidHex(startHex) ? ALU::hexToDec(startHex) : 0;
                loadProgramFile(fname, startAddr);
                break;
            }
            case 2: runAtOnce(); break;
            case 3: runStepByStep(); break;
            case 4: overwriteValue(); break;
            case 5: displayState(); break;
            case 6: resetMachine(); break;
            default: break;
        }
    }
}

void Machine::resetMachine() {
    cpu.reset(0);
    memory.clear();
    screenBuffer.clear();
}

void Machine::haltExecution() {
    cpu.halt();
}

const CPU& Machine::getCPU() const { return cpu; }
CPU& Machine::getCPU() { return cpu; }
const Memory& Machine::getMemory() const { return memory; }
Memory& Machine::getMemory() { return memory; }
const std::string& Machine::getScreenBuffer() const { return screenBuffer; }
