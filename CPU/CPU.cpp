#include <iostream>
#include "CPU.h"
#include <stdexcept>
#include <cctype>

CPU::CPU() : pc(0), ir("0000"), halted(false) {}

void CPU::fetch(const Memory& mem) {
    if (pc < 0 || pc + 1 >= mem.getSize() || pc % 2 != 0) {
        halted = true;
        return;
    }
    ir = mem.getWord(pc);
    pc += 2;
}

void CPU::decodeAndExecute(Memory& mem, string& screenBuffer) {
    char op = static_cast<char>(toupper(static_cast<unsigned char>(ir[0])));
    int R = ALU::hexToDec(string(1, ir[1]));
    string S = reg.getReg(ALU::hexToDec(string(1, ir[2])));
    string T = reg.getReg(ALU::hexToDec(string(1, ir[3])));
    string XY = ir.substr(2, 2);
    int addr = ALU::hexToDec(XY);

    switch (op) {
        case '1':
            reg.setReg(R, mem.getCell(addr));
            break;
        case '2':
            reg.setReg(R, XY);
            break;
        case '3':
            mem.setCell(addr, reg.getReg(R));
            if (addr == 0xFF || addr == 0x00) {
                screenBuffer += reg.getReg(R) + " ";
            }
            break;
        case '4':
            reg.setReg(ALU::hexToDec(string(1, ir[3])), S);
            break;
        case '5':
            reg.setReg(R, alu.addTwosComplement(S, T));
            break;
        case '6':
            reg.setReg(R, alu.addFloat(S, T));
            break;
        case '7':
            reg.setReg(R, alu.bitwiseOr(S, T));
            break;
        case '8':
            reg.setReg(R, alu.bitwiseAnd(S, T));
            break;
        case '9':
            reg.setReg(R, alu.bitwiseXor(S, T));
            break;
        case 'A':
            reg.setReg(R, alu.rotateRight(reg.getReg(R), ALU::hexToDec(string(1, ir[3]))));
            break;
        case 'B':
            if (reg.getReg(R) == reg.getReg(0)) pc = addr;
            break;
        case 'C':
            halted = true;
            break;
    }
}

bool CPU::isValidInstruction(string inst) {
    if (inst.size() != 4 || !ALU::isValidHex(inst)) return false;
    for (char& c : inst) {
        c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }
    char op = inst[0];
    if (!((op >= '1' && op <= '9') || (op >= 'A' && op <= 'C'))) return false;
    if (op == '4' && inst[1] != '0') return false;
    if (op == 'A' && inst[2] != '0') return false;
    if (op == 'C' && inst != "C000") return false;
    return true;
}

bool CPU::runNextStep(Memory& mem, string& screenBuffer) {
    if (halted) return false;
    fetch(mem);
    if (halted || !isValidInstruction(ir)) {
        halted = true;
        return false;
    }
    decodeAndExecute(mem, screenBuffer);
    return !halted;
}

void CPU::setPC(int addr) {
    pc = addr;
    halted = false;
}

int CPU::getPC() const {
    return pc;
}

string CPU::getIR() const {
    return ir;
}

bool CPU::isHalted() const {
    return halted;
}

const Register& CPU::getRegister() const {
    return reg;
}

void CPU::setRegister(int index, string val) {
    reg.setReg(index, val);
}

void CPU::reset(int startAddress) {
    pc = startAddress;
    ir = "0000";
    halted = false;
    reg.clear();
}
