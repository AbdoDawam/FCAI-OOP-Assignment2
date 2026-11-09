#include "Storage.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

string Storage::normalizeHexValue(const string& val) const {
    string normalized = val;
    transform(normalized.begin(), normalized.end(), normalized.begin(),
              [](unsigned char c) { return static_cast<char>(toupper(c)); });

    while (normalized.length() < 2) {
        normalized = "0" + normalized;
    }
    if (normalized.length() > 2) {
        normalized = normalized.substr(normalized.length() - 2);
    }

    return normalized;
}

Storage::Storage(int s) : size(s), cells(s, "00") {}

string Storage::getCell(int index) const {
    if (index >= 0 && index < size) {
        return cells[index];
    }
    cout << "Index out of bounds" << endl;
    return "00";
}

void Storage::setCell(int index, string val) {
    if (index >= 0 && index < size) {
        cells[index] = normalizeHexValue(val);
    } else {
        cout << "Index out of bounds" << endl;
    }
}

void Storage::clear() {
    fill(cells.begin(), cells.end(), "00");
}

int Storage::getSize() const {
    return size;
}

Storage::~Storage() = default;
