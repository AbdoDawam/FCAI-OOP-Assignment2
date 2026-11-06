/*
 *بسم الله الرحمن الرحيم
 * File:   ALU.cpp
 * Date: 6 Oct 2026
 * Purpose: Implementation of the ALU class of the Vole machine simulator.
 */
#include "ALU.h"
#include <cmath>
#include <stdexcept>

namespace {
    int digitValue(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    }

    int toByte(const std::string& s) {
        if (!ALU::isValidHex(s))
            throw std::invalid_argument("ALU: invalid hex byte '" + s + "'");
        return ALU::hexToDec(s);
    }
}

bool ALU::isValidHex(const std::string& str) {
    if (str.size() != 2) return false;
    return digitValue(str[0]) != -1 && digitValue(str[1]) != -1;
}

int ALU::hexToDec(const std::string& hex) {
    if (hex.empty()) return -1;
    int result = 0;
    for (char c : hex) {
        int d = digitValue(c);
        if (d == -1) return -1;
        result = result * 16 + d;
    }
    return result;
}

std::string ALU::decToHex(int dec, int width) {
    const char* digits = "0123456789ABCDEF";
    unsigned int v = static_cast<unsigned int>(dec);
    std::string out(width, '0');
    for (int i = width - 1; i >= 0; --i) {
        out[i] = digits[v & 0xF];
        v >>= 4;
    }
    return out;
}

std::string ALU::addTwosComplement(const std::string& val1, const std::string& val2) const {
    int sum = toByte(val1) + toByte(val2);
    return decToHex(sum & 0xFF, 2);
}

std::string ALU::bitwiseOr(const std::string& val1, const std::string& val2) const {
    return decToHex((toByte(val1) | toByte(val2)) & 0xFF, 2);
}

std::string ALU::bitwiseAnd(const std::string& val1, const std::string& val2) const {
    return decToHex((toByte(val1) & toByte(val2)) & 0xFF, 2);
}

std::string ALU::bitwiseXor(const std::string& val1, const std::string& val2) const {
    return decToHex((toByte(val1) ^ toByte(val2)) & 0xFF, 2);
}

std::string ALU::rotateRight(const std::string& val, int steps) const {
    int v = toByte(val);
    int s = ((steps % 8) + 8) % 8;
    int result = ((v >> s) | (v << (8 - s))) & 0xFF;
    return decToHex(result, 2);
}

double ALU::floatToDec(const std::string& hexByte) const {
    int b = toByte(hexByte);
    int sign = (b >> 7) & 1;
    int exp = (b >> 4) & 0x7;
    int mant = b & 0xF;
    double v = (mant / 16.0) * std::pow(2.0, exp - 4);
    return sign ? -v : v;
}

std::string ALU::decToFloat(double value) const {
    if (value == 0.0) return "00";
    int sign = (value < 0) ? 1 : 0;
    double a = std::fabs(value);
    int e = 0;
    while (a >= 1.0) { a /= 2.0; ++e; }
    while (a < 0.5) { a *= 2.0; --e; }

    int expField = e + 4;
    int mant;
    if (expField > 7) { expField = 7; mant = 15; }
    else if (expField < 0) { return "00"; }
    else { mant = static_cast<int>(a * 16.0); }

    return decToHex((sign << 7) | (expField << 4) | mant, 2);
}

std::string ALU::addFloat(const std::string& val1, const std::string& val2) const {
    return decToFloat(floatToDec(val1) + floatToDec(val2));
}
