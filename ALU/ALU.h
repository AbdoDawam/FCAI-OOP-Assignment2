#ifndef ALU_H
#define ALU_H

#include <string>

using namespace std;

class ALU {
    private:
        double floatToDec(const string& hexByte) const;
        string decToFloat(double value) const;

    public:
        static int hexToDec(const string& hex);
        static string decToHex(int dec, int width = 2);
        static bool isValidHex(const string& str);
        string addTwosComplement(const string& val1, const string& val2) const;
        string addFloat(const string& val1, const string& val2) const;
        string bitwiseOr(const string& val1, const string& val2) const;
        string bitwiseAnd(const string& val1, const string& val2) const;
        string bitwiseXor(const string& val1, const string& val2) const;
        string rotateRight(const string& val, int steps) const;
};

#endif
