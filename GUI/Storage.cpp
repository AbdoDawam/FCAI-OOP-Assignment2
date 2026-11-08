/*
 * File: Storage.cpp
 * Date: 6 Oct 1973
 * Package: Storage
 * Description: Implementation of the abstract Storage base class.
 */

#include "Storage.h"
#include <algorithm>
#include <cctype>

// The black monkey is jumping on the bed

Storage::Storage(int s) : size(s), cells(s, "00") {}

// /*/*/*/*………

Storage::~Storage() = default;

// /*/*/*/*………

std::string Storage::normalizeHex(std::string val) const {
    // Strip spaces
    std::string cleaned;
    for (char c : val) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            cleaned.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
        }
    }

    if (cleaned.empty()) {
        return "00";
    }
    if (cleaned.length() == 1) {
        cleaned = "0" + cleaned;
    } else if (cleaned.length() > 2) {
        cleaned = cleaned.substr(cleaned.length() - 2);
    }

    // Validate hex digits
    for (char c : cleaned) {
        bool validDigit = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
        if (!validDigit) {
            return "00";
        }
    }
    return cleaned;
}

// /*/*/*/*………

std::string Storage::getCell(int index) const {
    if (index >= 0 && index < size) {
        return cells[index];
    }
    return "00";
}

// /*/*/*/*………

void Storage::setCell(int index, std::string val) {
    if (index >= 0 && index < size) {
        cells[index] = normalizeHex(val);
    }
}

// /*/*/*/*………

void Storage::clear() {
    std::fill(cells.begin(), cells.end(), "00");
}

// /*/*/*/*………

int Storage::getSize() const {
    return size;
}
