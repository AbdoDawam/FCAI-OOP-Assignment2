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

    static int hexToSignedTwosComplement(const std::string& hexByte);
};

#endif
