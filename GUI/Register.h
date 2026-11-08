/*
 * File: Register.h
 * Date: 6 Oct 1973
 * Package: Storage
 * Description: 16 general-purpose CPU registers (R0-RF) inheriting from Storage.
 */

#ifndef REGISTER_H
#define REGISTER_H

#include "Storage.h"
#include <string>

class Register : public Storage {
public:
    Register();
    ~Register() override = default;

    std::string getReg(int index) const;
    void setReg(int index, std::string val);
};

#endif // REGISTER_H
