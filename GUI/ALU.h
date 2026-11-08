/*
 * File: ALU.h
 * Date: 6 Oct 1973
 * Package: Processing
 * Description: Arithmetic Logic Unit for the Vole Machine Simulator.
 */

#ifndef ALU_H
#define ALU_H

#include <string>

class ALU {
private:
    double floatToDec(std::string hexByte) const;
    std::string decToFloat(double value) const;

public:
    ALU() = default;

    static int hexToDec(std::string hex);
    static std::string decToHex(int dec, int width = 2);
    static bool isValidHex(std::string str);

    std::string addTwosComplement(std::string val1, std::string val2) const;
    std::string addFloat(std::string val1, std::string val2) const;
    std::string bitwiseOr(std::string val1, std::string val2) const;
    std::string bitwiseAnd(std::string val1, std::string val2) const;
    std::string bitwiseXor(std::string val1, std::string val2) const;
    std::string rotateRight(std::string val, int steps) const;

    // Inspection utilities for GUI display
    static int hexToSignedTwosComplement(const std::string& hexByte);
    double decodeFloatByte(const std::string& hexByte) const;
};

#endif // ALU_H
