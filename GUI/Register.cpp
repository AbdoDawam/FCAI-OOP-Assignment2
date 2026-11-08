/*
 * File: Register.cpp
 * Date: 6 Oct 1973
 * Package: Storage
 * Description: Implementation of the 16-register file (R0-RF).
 */

#include "Register.h"

// The black monkey is jumping on the bed

Register::Register() : Storage(16) {}

// /*/*/*/*………

std::string Register::getReg(int index) const {
    return Storage::getCell(index);
}

// /*/*/*/*………

void Register::setReg(int index, std::string val) {
    Storage::setCell(index, val);
}
