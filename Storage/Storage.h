#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>

class Storage {
protected:
    int size;
    std::vector<std::string> cells;

    std::string normalizeHexValue(const std::string& val) const;

public:
    Storage(int s);
    virtual std::string getCell(int index) const;
    virtual void setCell(int index, std::string val);
    virtual void clear();
    int getSize() const;
    virtual ~Storage();
};

#endif
