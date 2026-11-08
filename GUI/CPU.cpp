/*
 * File: CPU.cpp
 * Date: 6 Oct 1973
 * Package: Processing
 * Description: Implementation of CPU instruction cycle (fetch, decode, execute).
 */

#include "CPU.h"
#include <algorithm>
#include <cctype>
#include <sstream>

// The black monkey is jumping on the bed

CPU::CPU()
    : pc(0),
      ir("0000"),
      halted(false),
      lastExecutedPC(-1),
      lastModifiedReg(-1),
      lastWrittenMem(-1),
      stepCount(0),
      lastActionDescription("CPU Initialized (Ready)") {}

// /*/*/*/*………

void CPU::fetch(const Memory& mem) {
    if (pc < 0 || pc > 254) {
        ir = "0000";
        halted = true;
        lastActionDescription = "PC out of bounds (" + ALU::decToHex(pc, 2) + ") - Halted";
        return;
    }
    lastExecutedPC = pc;
    ir = mem.getWord(pc);
    pc += 2;
}

// /*/*/*/*………

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

// /*/*/*/*………

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
        case '1': { // 1RXY: LOAD R from Memory[XY]
            std::string val = mem.getCell(addr);
            reg.setReg(rIdx, val);
            lastModifiedReg = rIdx;
            desc << "LOAD R" << ir[1] << " <- Mem[0x" << xy << "] (0x" << val << ")";
            break;
        }
        case '2': { // 2RXY: LOAD R with immediate XY
            reg.setReg(rIdx, xy);
            lastModifiedReg = rIdx;
            desc << "LOAD R" << ir[1] << " <- Immediate 0x" << xy;
            break;
        }
        case '3': { // 3RXY: STORE R into Memory[XY] (FF = Screen)
            std::string val = reg.getReg(rIdx);
            mem.setCell(addr, val);
            lastWrittenMem = addr;
            if (addr == 0xFF) {
                int byteVal = ALU::hexToDec(val);
                if (byteVal >= 32 && byteVal <= 126) {
                    screenBuffer.push_back(static_cast<char>(byteVal));
                } else if (byteVal == 10 || byteVal == 13) {
                    screenBuffer.push_back('\n');
                } else {
                    screenBuffer += "[" + val + "h=" + std::to_string(byteVal) + "] ";
                }
                screenHexHistory.push_back(val);
                desc << "STORE R" << ir[1] << " (0x" << val << ") -> Screen [0xFF]";
            } else {
                desc << "STORE Mem[0x" << xy << "] <- R" << ir[1] << " (0x" << val << ")";
            }
            break;
        }
        case '4': { // 40RS: MOVE R (ir[2]) to S (ir[3])
            std::string val = reg.getReg(sIdx);
            reg.setReg(tIdx, val);
            lastModifiedReg = tIdx;
            desc << "MOVE R" << ir[3] << " <- R" << ir[2] << " (0x" << val << ")";
            break;
        }
        case '5': { // 5RST: ADD Two's Complement R <- S + T
            std::string vS = reg.getReg(sIdx);
            std::string vT = reg.getReg(tIdx);
            std::string res = alu.addTwosComplement(vS, vT);
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "ADD(2's) R" << ir[1] << " <- R" << ir[2] << "(0x" << vS << ") + R" << ir[3]
                 << "(0x" << vT << ") = 0x" << res;
            break;
        }
        case '6': { // 6RST: ADD Float R <- S + T
            std::string vS = reg.getReg(sIdx);
            std::string vT = reg.getReg(tIdx);
            std::string res = alu.addFloat(vS, vT);
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "ADD(Float) R" << ir[1] << " <- R" << ir[2] << "(0x" << vS << ") + R" << ir[3]
                 << "(0x" << vT << ") = 0x" << res;
            break;
        }
        case '7': { // 7RST: OR R <- S | T
            std::string vS = reg.getReg(sIdx);
            std::string vT = reg.getReg(tIdx);
            std::string res = alu.bitwiseOr(vS, vT);
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "OR R" << ir[1] << " <- R" << ir[2] << "(0x" << vS << ") | R" << ir[3]
                 << "(0x" << vT << ") = 0x" << res;
            break;
        }
        case '8': { // 8RST: AND R <- S & T
            std::string vS = reg.getReg(sIdx);
            std::string vT = reg.getReg(tIdx);
            std::string res = alu.bitwiseAnd(vS, vT);
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "AND R" << ir[1] << " <- R" << ir[2] << "(0x" << vS << ") & R" << ir[3]
                 << "(0x" << vT << ") = 0x" << res;
            break;
        }
        case '9': { // 9RST: XOR R <- S ^ T
            std::string vS = reg.getReg(sIdx);
            std::string vT = reg.getReg(tIdx);
            std::string res = alu.bitwiseXor(vS, vT);
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "XOR R" << ir[1] << " <- R" << ir[2] << "(0x" << vS << ") ^ R" << ir[3]
                 << "(0x" << vT << ") = 0x" << res;
            break;
        }
        case 'A': { // AR0X: ROTATE R right by X bits
            std::string vR = reg.getReg(rIdx);
            std::string res = alu.rotateRight(vR, tIdx);
            reg.setReg(rIdx, res);
            lastModifiedReg = rIdx;
            desc << "ROTATE R" << ir[1] << " (0x" << vR << ") right by " << tIdx << " -> 0x" << res;
            break;
        }
        case 'B': { // BRXY: JUMP to XY if R == R0
            std::string vR = reg.getReg(rIdx);
            std::string v0 = reg.getReg(0);
            if (vR == v0) {
                pc = addr;
                desc << "JUMP taken to 0x" << xy << " (R" << ir[1] << " == R0 == 0x" << v0 << ")";
            } else {
                desc << "JUMP not taken (R" << ir[1] << "[0x" << vR << "] != R0[0x" << v0 << "])";
            }
            break;
        }
        case 'C': { // C000: HALT
            halted = true;
            desc << "HALT execution (C000)";
            break;
        }
        default: {
            halted = true;
            desc << "Invalid Opcode '" << op << "' - Halted";
            break;
        }
    }

    lastActionDescription = desc.str();
}

// /*/*/*/*………

bool CPU::runNextStep(Memory& mem, std::string& screenBuffer) {
    if (halted) {
        return false;
    }
    fetch(mem);
    if (halted) {
        return false;
    }
    if (!isValidInstruction(ir)) {
        halted = true;
        lastActionDescription = "Invalid instruction '" + ir + "' at 0x" +
                                ALU::decToHex(lastExecutedPC, 2) + " - Halted";
        return false;
    }
    stepCount++;
    decodeAndExecute(mem, screenBuffer);
    return !halted;
}

// /*/*/*/*………

void CPU::setPC(int addr) {
    if (addr >= 0 && addr < 256) {
        pc = addr;
        halted = false;
    }
}

// /*/*/*/*………

int CPU::getPC() const {
    return pc;
}

// /*/*/*/*………

std::string CPU::getIR() const {
    return ir;
}

// /*/*/*/*………

bool CPU::isHalted() const {
    return halted;
}

// /*/*/*/*………

void CPU::halt() {
    halted = true;
    lastActionDescription = "CPU Halted by user.";
}

// /*/*/*/*………

const Register& CPU::getRegisters() const {
    return reg;
}

// /*/*/*/*………

void CPU::setRegister(int index, std::string val) {
    reg.setReg(index, val);
    lastModifiedReg = index;
}

// /*/*/*/*………

void CPU::reset(int startAddr) {
    pc = (startAddr >= 0 && startAddr < 256) ? startAddr : 0;
    ir = "0000";
    halted = false;
    reg.clear();
    lastExecutedPC = -1;
    lastModifiedReg = -1;
    lastWrittenMem = -1;
    stepCount = 0;
    screenHexHistory.clear();
    lastActionDescription = "CPU Reset (PC = 0x" + ALU::decToHex(pc, 2) + ")";
}

// /*/*/*/*………

const ALU& CPU::getALU() const {
    return alu;
}

int CPU::getLastExecutedPC() const {
    return lastExecutedPC;
}

int CPU::getLastModifiedReg() const {
    return lastModifiedReg;
}

int CPU::getLastWrittenMem() const {
    return lastWrittenMem;
}

int CPU::getStepCount() const {
    return stepCount;
}

std::string CPU::getLastActionDescription() const {
    return lastActionDescription;
}

const std::vector<std::string>& CPU::getScreenHexHistory() const {
    return screenHexHistory;
}

// /*/*/*/*………

std::string CPU::disassembleInstruction(const std::string& inst) {
    if (inst.size() != 4 || !ALU::isValidHex(inst)) {
        return "DATA / INVALID";
    }
    std::string u = inst;
    for (char& c : u) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    if (u == "0000") return "NOP / Empty (0000)";
    if (!isValidInstruction(u)) return "Invalid Instruction (" + u + ")";

    char op = u[0];
    char r = u[1];
    char s = u[2];
    char t = u[3];
    std::string xy = u.substr(2, 2);

    std::ostringstream out;
    switch (op) {
        case '1': out << "LOAD R" << r << " <- Mem[0x" << xy << "]"; break;
        case '2': out << "LOAD R" << r << " <- 0x" << xy << " (Imm)"; break;
        case '3':
            if (xy == "FF") out << "SCREEN [0xFF] <- R" << r;
            else out << "STORE Mem[0x" << xy << "] <- R" << r;
            break;
        case '4': out << "MOVE R" << t << " <- R" << s; break;
        case '5': out << "ADD(2's) R" << r << " <- R" << s << " + R" << t; break;
        case '6': out << "ADD(Flt) R" << r << " <- R" << s << " + R" << t; break;
        case '7': out << "OR  R" << r << " <- R" << s << " | R" << t; break;
        case '8': out << "AND R" << r << " <- R" << s << " & R" << t; break;
        case '9': out << "XOR R" << r << " <- R" << s << " ^ R" << t; break;
        case 'A': out << "ROR R" << r << " >> " << ALU::hexToDec(std::string(1, t)) << " bits"; break;
        case 'B': out << "JMP 0x" << xy << " if R" << r << " == R0"; break;
        case 'C': out << "HALT Execution"; break;
        default:  out << "Unknown"; break;
    }
    return out.str();
}
