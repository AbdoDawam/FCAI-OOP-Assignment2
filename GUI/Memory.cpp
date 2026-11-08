#include "Memory.h"
#include <cctype>

Memory::Memory() : Storage(256) {}

std::string Memory::getCell(int address) const {
    return Storage::getCell(address);
}

void Memory::setCell(int address, std::string val) {
    std::string trimmed;
    for (char c : val) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            trimmed.push_back(c);
        }
    }

    if (trimmed.length() == 4) {
        Storage::setCell(address, trimmed.substr(0, 2));
        if (address + 1 < size) {
            Storage::setCell(address + 1, trimmed.substr(2, 2));
        }
    } else {
        Storage::setCell(address, trimmed);
    }
}

std::string Memory::getWord(int address) const {
    if (address < 0 || address >= size - 1) {
        return "0000";
    }
    return getCell(address) + getCell(address + 1);
}
