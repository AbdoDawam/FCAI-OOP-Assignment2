#include "CPU.h"
#include <cctype>
#include <sstream>

CPU::CPU()
    : pc(0),
      ir("0000"),
      halted(false),
      lastExecutedPC(-1),
      lastModifiedReg(-1),
      lastWrittenMem(-1),
      lastActionDescription("Ready") {}

void CPU::fetch(const Memory& mem) {
    if (pc < 0 || pc > 254) {
        ir = "0000";
        halted = true;
        lastActionDescription = "PC out of bounds - Halted";
        return;
    }
    lastExecutedPC = pc;
    ir = mem.getWord(pc);
    pc += 2;
}

bool CPU::isValidInstruction(std::string inst) {
    if (inst.size() != 4) return false;
    for (char& c : inst) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    if (!ALU::isValidHex(inst)) return false;

    char op = inst[0];
    bool validOp = (op >= '1' && op <= '9') || (op >= 'A' && op <= 'C');
    if (!validOp) return false;

    if (op == '4' && inst[1] != '0') return false;
    if (op == 'A' && inst[2] != '0') return false;
    if (op == 'C' && inst != "C000") return false;

    return true;
}

void CPU::decodeAndExecute(Memory& mem, std::string& screenBuffer) {
    lastModifiedReg = -1;
    lastWrittenMem = -1;

    if (ir.size() < 4) {
        halted = true;
        return;
    }

    char op = static_cast<char>(std::toupper(static_cast<unsigned char>(ir[0])));
    int rIdx = ALU::hexToDec(std::string(1, ir[1]));
    int sIdx = ALU::hexToDec(std::string(1, ir[2]));
    int tIdx = ALU::hexToDec(std::string(1, ir[3]));
    std::string xy = ir.substr(2, 2);
    int addr = ALU::hexToDec(xy);

    std::ostringstream desc;

    switch (op) {
        case '1': {
            std::string val = mem.getCell(addr);
            reg.setReg(rIdx, val);
            lastModifiedReg = rIdx;
            desc << "LOAD R" << ir[1] << " <- Mem[" << xy << "]";
            break;
        }
        case '2': {
            reg.setReg(rIdx, xy);
            lastModifiedReg = rIdx;
            desc << "LOAD R" << ir[1] << " <- " << xy;
            break;
        }
        case '3': {
            std::string val = reg.getReg(rIdx);
            mem.setCell(addr, val);
            lastWrittenMem = addr;
            if (addr == 0xFF) {
                int byteVal = ALU::hexToDec(val);
                if (byteVal >= 32 && byteVal <= 126) {
                    screenBuffer.push_back(static_cast<char>(byteVal));
                } else {
                    screenBuffer += val + " ";
                }
                desc << "STORE R" << ir[1] << " -> Screen [FF]";
            } else {
                desc << "STORE Mem[" << xy << "] <- R" << ir[1];
            }
            break;
        }
        case '4': {
            std::string val = reg.getReg(sIdx);
            reg.setReg(tIdx, val);
            lastModifiedReg = tIdx;
            desc << "MOVE R" << ir[3] << " <- R" << ir[2];
            break;
        }
        case '5': {
            std::string res = alu.addTwosComplement(reg.getReg(sIdx), reg.getReg(tIdx));
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "ADD (2's) R" << ir[1] << " <- R" << ir[2] << " + R" << ir[3];
            break;
        }
        case '6': {
            std::string res = alu.addFloat(reg.getReg(sIdx), reg.getReg(tIdx));
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "ADD (Float) R" << ir[1] << " <- R" << ir[2] << " + R" << ir[3];
            break;
        }
        case '7': {
            std::string res = alu.bitwiseOr(reg.getReg(sIdx), reg.getReg(tIdx));
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "OR R" << ir[1] << " <- R" << ir[2] << " | R" << ir[3];
            break;
        }
        case '8': {
            std::string res = alu.bitwiseAnd(reg.getReg(sIdx), reg.getReg(tIdx));
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "AND R" << ir[1] << " <- R" << ir[2] << " & R" << ir[3];
            break;
        }
        case '9': {
            std::string res = alu.bitwiseXor(reg.getReg(sIdx), reg.getReg(tIdx));
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "XOR R" << ir[1] << " <- R" << ir[2] << " ^ R" << ir[3];
            break;
        }
        case 'A': {
            std::string res = alu.rotateRight(reg.getReg(rIdx), tIdx);
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "ROTATE R" << ir[1] << " right by " << tIdx;
            break;
        }
        case 'B': {
            if (reg.getReg(rIdx) == reg.getReg(0)) {
                pc = addr;
                desc << "JUMP to " << xy;
            } else {
                desc << "JUMP not taken";
            }
            break;
        }
        case 'C': {
            halted = true;
            desc << "HALT";
            break;
        }
        default: {
            halted = true;
            desc << "Invalid Opcode - Halted";
            break;
        }
    }

    lastActionDescription = desc.str();
}

bool CPU::runNextStep(Memory& mem, std::string& screenBuffer) {
    if (halted) return false;
    fetch(mem);
    if (halted) return false;
    if (!isValidInstruction(ir)) {
        halted = true;
        lastActionDescription = "Invalid instruction '" + ir + "' - Halted";
        return false;
    }
    decodeAndExecute(mem, screenBuffer);
    return !halted;
}

void CPU::setPC(int addr) {
    if (addr >= 0 && addr < 256) {
        pc = addr;
        halted = false;
    }
}

int CPU::getPC() const { return pc; }
std::string CPU::getIR() const { return ir; }
bool CPU::isHalted() const { return halted; }

void CPU::halt() {
    halted = true;
    lastActionDescription = "Halted";
}

const Register& CPU::getRegisters() const { return reg; }

void CPU::setRegister(int index, std::string val) {
    reg.setReg(index, val);
    lastModifiedReg = index;
}

void CPU::reset(int startAddr) {
    pc = (startAddr >= 0 && startAddr < 256) ? startAddr : 0;
    ir = "0000";
    halted = false;
    reg.clear();
    lastExecutedPC = -1;
    lastModifiedReg = -1;
    lastWrittenMem = -1;
    lastActionDescription = "Reset";
}

int CPU::getLastModifiedReg() const { return lastModifiedReg; }
int CPU::getLastWrittenMem() const { return lastWrittenMem; }
std::string CPU::getLastActionDescription() const { return lastActionDescription; }
