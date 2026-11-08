/*
 * File: ALU.h
 * Date: 6 Oct 1973
 */
#ifndef ALU_H
#define ALU_H

#include <string>

class ALU {
public:
    static int hexToDec(const std::string& hex);

    // /*/*/*/*……

    static std::string decToHex(int dec, int width = 2);

    // /*/*/*/*……

    static bool isValidHex(const std::string& str);

    // /*/*/*/*……

    std::string addTwosComplement(const std::string& val1, const std::string& val2) const;

    // /*/*/*/*……

    std::string addFloat(const std::string& val1, const std::string& val2) const;

    // /*/*/*/*……

    std::string bitwiseOr(const std::string& val1, const std::string& val2) const;

    // /*/*/*/*……

    std::string bitwiseAnd(const std::string& val1, const std::string& val2) const;

    // /*/*/*/*……

    std::string bitwiseXor(const std::string& val1, const std::string& val2) const;

    // /*/*/*/*……

    std::string rotateRight(const std::string& val, int steps) const;

private:
    double floatToDec(const std::string& hexByte) const;

    // /*/*/*/*……

    std::string decToFloat(double value) const;
};

#endif
