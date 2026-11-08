/*
 * File: Machine.cpp
 * Date: 6 Oct 1973
 * Package: Orchestration
 * Description: Implementation of the Machine orchestrator class.
 */

#include "Machine.h"
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

// The black monkey is jumping on the bed

Machine::Machine() {
    resetMachine();
}

// /*/*/*/*………

bool Machine::loadProgramFromText(const std::string& sourceText, int startAddr,
                                  std::string& outMessage, int& outInstructionCount) {
    outInstructionCount = 0;

    if (startAddr < 0 || startAddr >= 255 || (startAddr % 2 != 0)) {
        outMessage = "Error: Start address must be an even address between 00 and FE.";
        return false;
    }

    std::istringstream stream(sourceText);
    std::string line;
    std::vector<std::string> instructions;
    int lineNum = 0;

    while (std::getline(stream, line)) {
        lineNum++;
        // Strip comments starting with ';'
        std::size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Extract first non-whitespace token
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
            outMessage = "Line " + std::to_string(lineNum) + ": Instruction '" + cleaned +
                         "' is shorter than 4 hex digits.";
            return false;
        }

        std::string inst = cleaned.substr(0, 4);
        if (!CPU::isValidInstruction(inst)) {
            outMessage = "Line " + std::to_string(lineNum) + ": Invalid Vole instruction '" +
                         inst + "' (Program rejected).";
            return false;
        }

        instructions.push_back(inst);
    }

    if (instructions.empty()) {
        outMessage = "Error: No valid instructions found in program.";
        return false;
    }

    if (startAddr + static_cast<int>(instructions.size()) * 2 > 256) {
        outMessage = "Error: Program exceeds 256-byte memory capacity from start address 0x" +
                     ALU::decToHex(startAddr, 2) + ".";
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
                 " instructions at 0x" + ALU::decToHex(startAddr, 2) + ".";
    addLogEntry(outMessage);
    return true;
}

// /*/*/*/*………

bool Machine::loadProgramFileWithReport(const std::string& filename, int startAddr,
                                        std::string& outMessage, int& outInstructionCount,
                                        std::string& outSourceText) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        outMessage = "Error: Could not open file '" + filename + "'.";
        outInstructionCount = 0;
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    outSourceText = buffer.str();
    file.close();

    return loadProgramFromText(outSourceText, startAddr, outMessage, outInstructionCount);
}

// /*/*/*/*………

bool Machine::loadProgramFile(std::string filename, int startAddr) {
    std::string msg;
    std::string src;
    int count = 0;
    return loadProgramFileWithReport(filename, startAddr, msg, count, src);
}

// /*/*/*/*………

bool Machine::executeSingleStep() {
    if (cpu.isHalted()) {
        return false;
    }
    int fetchPC = cpu.getPC();
    bool cont = cpu.runNextStep(memory, screenBuffer);
    std::ostringstream logLine;
    logLine << "[Step " << cpu.getStepCount() << " | PC 0x" << ALU::decToHex(fetchPC, 2)
            << " | IR " << cpu.getIR() << "] " << cpu.getLastActionDescription();
    addLogEntry(logLine.str());
    return cont;
}

// /*/*/*/*………

int Machine::executeAllAtOnce(int maxSteps) {
    int executed = 0;
    while (!cpu.isHalted() && executed < maxSteps) {
        executeSingleStep();
        executed++;
    }
    if (!cpu.isHalted() && executed >= maxSteps) {
        addLogEntry("Execution paused: Reached maxSteps (" + std::to_string(maxSteps) +
                    ") safeguard.");
    }
    return executed;
}

// /*/*/*/*………

void Machine::runAtOnce(int maxSteps) {
    executeAllAtOnce(maxSteps);
    displayState();
}

// /*/*/*/*………

void Machine::runStepByStep() {
    while (!cpu.isHalted()) {
        executeSingleStep();
        displayState();
        if (cpu.isHalted()) {
            std::cout << "Program halted.\n";
            break;
        }
        std::cout << "Press Enter to execute next step...";
        std::cin.ignore(10000, '\n');
    }
}

// /*/*/*/*………

void Machine::overwriteValue() {
    int choice = 0;
    std::cout << "Overwrite (1 = Memory Cell, 2 = Register): ";
    std::cin >> choice;
    if (choice == 1) {
        std::string addrHex, valHex;
        std::cout << "Enter Memory Address (Hex 00-FF): ";
        std::cin >> addrHex;
        std::cout << "Enter New Value (Hex 00-FF): ";
        std::cin >> valHex;
        if (ALU::isValidHex(addrHex) && ALU::isValidHex(valHex)) {
            int addr = ALU::hexToDec(addrHex);
            memory.setCell(addr, valHex);
        }
    } else if (choice == 2) {
        std::string regHex, valHex;
        std::cout << "Enter Register Index (Hex 0-F): ";
        std::cin >> regHex;
        std::cout << "Enter New Value (Hex 00-FF): ";
        std::cin >> valHex;
        if (ALU::isValidHex(regHex) && ALU::isValidHex(valHex)) {
            int rIdx = ALU::hexToDec(regHex);
            cpu.setRegister(rIdx, valHex);
        }
    }
}

// /*/*/*/*………

void Machine::displayState() const {
    std::cout << "\n==================== VOLE MACHINE STATE ====================\n";
    std::cout << "PC: 0x" << ALU::decToHex(cpu.getPC(), 2)
              << " | IR: " << cpu.getIR()
              << " | Halted: " << (cpu.isHalted() ? "YES" : "NO") << "\n\n";

    std::cout << "Registers:\n";
    const Register& regs = cpu.getRegisters();
    for (int i = 0; i < 16; ++i) {
        std::cout << "R" << ALU::decToHex(i, 1) << ": " << regs.getReg(i)
                  << ((i % 4 == 3) ? "\n" : "   ");
    }

    std::cout << "\nMemory Matrix (16x16):\n    ";
    for (int c = 0; c < 16; ++c) std::cout << " " << ALU::decToHex(c, 1) << " ";
    std::cout << "\n";
    for (int r = 0; r < 16; ++r) {
        std::cout << " " << ALU::decToHex(r, 1) << "_ ";
        for (int c = 0; c < 16; ++c) {
            std::cout << memory.getCell(r * 16 + c) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\nScreen Output [0xFF]: " << screenBuffer << "\n";
    std::cout << "============================================================\n";
}

// /*/*/*/*………

void Machine::displayMenu() {
    int option = 0;
    while (option != 7) {
        std::cout << "\n========= VOLE MACHINE SIMULATOR =========\n"
                  << "1. Load Program from File\n"
                  << "2. Run Program At Once\n"
                  << "3. Run Program Step-by-Step\n"
                  << "4. Overwrite Memory / Register\n"
                  << "5. Display Machine State\n"
                  << "6. Reset Machine\n"
                  << "7. Exit\n"
                  << "Select option: ";
        if (!(std::cin >> option)) break;

        switch (option) {
            case 1: {
                std::string fname, startHex;
                std::cout << "Enter filename: ";
                std::cin >> fname;
                std::cout << "Enter even start address in Hex (e.g. 00): ";
                std::cin >> startHex;
                int startAddr = ALU::isValidHex(startHex) ? ALU::hexToDec(startHex) : 0;
                if (loadProgramFile(fname, startAddr)) {
                    std::cout << "Program loaded successfully.\n";
                } else {
                    std::cout << "Failed to load program.\n";
                }
                break;
            }
            case 2:
                runAtOnce();
                break;
            case 3:
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
                std::cout << "Machine reset complete.\n";
                break;
            default:
                break;
        }
    }
}

// /*/*/*/*………

void Machine::resetMachine() {
    cpu.reset(0);
    memory.clear();
    screenBuffer.clear();
    executionLog.clear();
    addLogEntry("Machine reset to initial state (Memory & Registers cleared to 00).");
}

// /*/*/*/*………

void Machine::haltExecution() {
    cpu.halt();
    addLogEntry("Execution manually halted by user.");
}

// /*/*/*/*………

void Machine::clearMemoryOnly() {
    memory.clear();
    addLogEntry("Main Memory (256 cells) cleared to 00.");
}

const CPU& Machine::getCPU() const { return cpu; }
CPU& Machine::getCPU() { return cpu; }
const Memory& Machine::getMemory() const { return memory; }
Memory& Machine::getMemory() { return memory; }
const std::string& Machine::getScreenBuffer() const { return screenBuffer; }
void Machine::clearScreenBuffer() { screenBuffer.clear(); }
const std::vector<std::string>& Machine::getExecutionLog() const { return executionLog; }

void Machine::addLogEntry(const std::string& entry) {
    executionLog.push_back(entry);
    if (executionLog.size() > 150) {
        executionLog.erase(executionLog.begin());
    }
}
