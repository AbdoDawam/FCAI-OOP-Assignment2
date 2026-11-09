#include "Register.h"

Register::Register() : Storage(16) {}

std::string Register::getReg(int index) const {
    return Storage::getCell(index);
}

void Register::setReg(int index, std::string val) {
    Storage::setCell(index, val);
}
