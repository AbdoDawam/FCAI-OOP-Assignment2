/*
 * File: Memory.h
 * Date: 6 Oct 1973
 * Package: Storage
 * Description: 256-cell main memory unit inheriting from Storage.
 */

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

#endif // MEMORY_H
