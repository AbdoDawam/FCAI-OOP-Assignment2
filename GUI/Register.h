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

#endif
