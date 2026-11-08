#ifndef REGISTER_H
#define REGISTER_H

#include "../Storage/Storage.h"
#include <string>

class Register : public Storage
{
public:

    Register();
    std::string getReg(int index) const;
    void setReg(int index, const std::string& value);

};

#endif