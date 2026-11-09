#ifndef MEMORY_H
#define MEMORY_H

#include "../Storage/Storage.h"
#include <string>

using namespace std;

class Memory : public Storage {
public:
    Memory();
    string getWord(int address) const;
};

#endif
