#include "Register.h"
#include <stdexcept>

using namespace std;

Register::Register() : Storage(16) {}

string Register::getReg(int index) const {
    return getCell(index);
}

void Register::setReg(int index, const string& value) {
    setCell(index, value);
}
