#ifndef MEMORY_H
#define MEMORY_H

#include "../Storage/Storage.h"
#include <string>

class Memory : public Storage {
public:
    Memory();
    std::string getWord(int address) const;
};

#endif