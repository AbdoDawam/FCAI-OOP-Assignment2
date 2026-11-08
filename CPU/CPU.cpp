#include <iostream>
#include "CPU.h"


CPU::CPU() : pc(0), ir("0000"), halted(false) {}

void CPU::fetch(const Memory& mem) {
    ir = mem.getWord(pc);
    pc+=2;
}
void CPU::decodeAndExecute(Memory& mem, string& screenBuffer) {
    char op=ir[0];
    int R=ALU::hexToDec(string(1,ir[1]));
    int S=reg.getReg(ALU::hexToDec(string(1,ir[2])));
    int T=reg.getReg(ALU::hexToDec(string(1,ir[3])));
    string XY=ir.substr(2,2);
    int addr=ALU::hexToDec(XY);

    switch (op){
        case '1': reg.setReg(R, mem.getCell(addr)); break;
        case '2': reg.setReg(R,XY); break;
        case '3': 
        if (addr==0xFF) screenBuffer+= (char)ALU::hexToDec(reg.getReg(R));
        else mem.setCell(addr, reg.getReg(R));
        break;
        case '4': reg.setReg(ALU::hexToDec(string(1,ir[3])), reg.getReg(S)); break;
        case '5': reg.setReg(R, alu.addTwosComplement(reg.getReg(S), reg.getReg(T))); break;
        case '6': reg.setReg(R, alu.addFloat(S,T)); break;
        case '7': reg.setReg(R, alu.bitwiseOr(S,T)); break;
        case '8': reg.setReg(R, alu.bitwiseAnd(S,T)); break;
        case '9': reg.setReg(R, alu.bitwiseXor(S,T)); break;
        case 'A': reg.setReg(R, alu.rotateRight(reg.getReg(R),ALU::hexToDec(string(1,ir[3])))); break;
        case 'B': if (reg.getReg(R)== reg.getReg(0)) pc=addr; break;
        case 'C': halted=true; break;
    }
}

bool CPU::isValidInstruction(string inst) {
    if (inst.size()!=4 || !ALU::isValedHex(inst)) return false;
    char op=inst[0];
    if ( op<'1' || op>'C' ) return false;
    if ( op=='4' && inst[1]!='0' ) return false;
    if ( op=='A' && inst[2]!='0' ) return false;
    if ( op=='C' && inst!="C000" ) return false;
    return true;
}

bool CPU::runNextStep(Memory& mem, string& screenBuffer) {
    if (halted) return false;
    fetch(mem);
    if (!isValidInstruction(ir)) {
        halted=true;
        return false;
    }
    decodeAndExecute(mem, screenBuffer);
    return !halted;
}

void CPU::setPC(int addr){
    pc=addr;
}

int CPU::getPC()const {
    return pc;
}

string CPU::getIR()const {
    return ir;
}

bool CPU::isHalted()const {
    return halted;
}

const Register& CPU::getRegister()const {
    return reg;
}

void CPU::setRegister(int index, string val) {
    reg.setReg(index, val);
}
