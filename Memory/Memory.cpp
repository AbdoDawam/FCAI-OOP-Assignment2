#include "Memory.h"
#include <stdexcept>
using namespace std;


Memory::Memory() : Storage(256) {}

string Memory::getWord(int address) const {

    if (address < 0 || address + 1 >= getSize()) {
        throw out_of_range("Invalid memory address for word");
    }

    if (address % 2 != 0) {
        throw invalid_argument("Word address must be even");
    }

    string firstByte = getCell(address);
    string secondByte = getCell(address + 1);

    return firstByte + secondByte;
}