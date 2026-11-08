#ifndef MEMORY_H
#define MEMORY_H

#include "Storage.h"
#include <string>

class Memory : public Storage {
public:
    Memory();
    ~Memory() override = default;

    std::string getCell(int address) const override;
    void setCell(int address, std::string val) override;
    std::string getWord(int address) const;
};

#endif
