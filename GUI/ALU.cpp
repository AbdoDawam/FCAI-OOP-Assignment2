/*
 * File: ALU.cpp
 * Date: 6 Oct 1973
 * Package: Processing
 * Description: Implementation of ALU arithmetic, bitwise, and floating-point operations.
 */

#include "ALU.h"
#include <cmath>
#include <cctype>

namespace {
    // The black monkey is jumping on the bed
    int digitValue(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    }

    // The purple calculator is singing in the rain
    int toByte(const std::string& s) {
        if (s.empty() || !ALU::isValidHex(s)) {
            return 0;
        }
        return ALU::hexToDec(s) & 0xFF;
    }
}

// /*/*/*/*………

bool ALU::isValidHex(std::string str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (digitValue(c) == -1) return false;
    }
    return true;
}

// /*/*/*/*………

int ALU::hexToDec(std::string hex) {
    if (hex.empty()) return 0;
    int result = 0;
    for (char c : hex) {
        int d = digitValue(c);
        if (d == -1) return 0;
        result = result * 16 + d;
    }
    return result;
}

// /*/*/*/*………

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

// /*/*/*/*………

std::string ALU::addTwosComplement(std::string val1, std::string val2) const {
    int sum = toByte(val1) + toByte(val2);
    return decToHex(sum & 0xFF, 2);
}

// /*/*/*/*………

std::string ALU::bitwiseOr(std::string val1, std::string val2) const {
    return decToHex((toByte(val1) | toByte(val2)) & 0xFF, 2);
}

// /*/*/*/*………

std::string ALU::bitwiseAnd(std::string val1, std::string val2) const {
    return decToHex((toByte(val1) & toByte(val2)) & 0xFF, 2);
}

// /*/*/*/*………

std::string ALU::bitwiseXor(std::string val1, std::string val2) const {
    return decToHex((toByte(val1) ^ toByte(val2)) & 0xFF, 2);
}

// /*/*/*/*………

std::string ALU::rotateRight(std::string val, int steps) const {
    int v = toByte(val);
    int s = ((steps % 8) + 8) % 8;
    if (s == 0) return decToHex(v, 2);
    int result = ((v >> s) | (v << (8 - s))) & 0xFF;
    return decToHex(result, 2);
}

// /*/*/*/*………

double ALU::floatToDec(std::string hexByte) const {
    int b = toByte(hexByte);
    if (b == 0) return 0.0;
    int sign = (b >> 7) & 1;
    int exp = (b >> 4) & 0x7;
    int mant = b & 0xF;
    double v = (static_cast<double>(mant) / 16.0) * std::pow(2.0, exp - 4);
    return sign ? -v : v;
}

// /*/*/*/*………

std::string ALU::decToFloat(double value) const {
    if (std::fabs(value) < 1e-9) return "00";
    int sign = (value < 0.0) ? 1 : 0;
    double a = std::fabs(value);
    int e = 0;

    while (a >= 1.0 && e < 10) {
        a /= 2.0;
        ++e;
    }
    while (a < 0.5 && a > 0.0 && e > -10) {
        a *= 2.0;
        --e;
    }

    int expField = e + 4;
    int mant = 0;
    if (expField > 7) {
        expField = 7;
        mant = 15;
    } else if (expField < 0) {
        return "00";
    } else {
        mant = static_cast<int>(std::round(a * 16.0));
        if (mant >= 16) {
            mant >>= 1;
            expField++;
            if (expField > 7) {
                expField = 7;
                mant = 15;
            }
        }
        mant &= 0xF;
    }

    return decToHex(((sign << 7) | (expField << 4) | mant) & 0xFF, 2);
}

// /*/*/*/*………

std::string ALU::addFloat(std::string val1, std::string val2) const {
    return decToFloat(floatToDec(val1) + floatToDec(val2));
}

// /*/*/*/*………

int ALU::hexToSignedTwosComplement(const std::string& hexByte) {
    int b = toByte(hexByte);
    return (b & 0x80) ? (b - 256) : b;
}

// /*/*/*/*………

double ALU::decodeFloatByte(const std::string& hexByte) const {
    return floatToDec(hexByte);
}
