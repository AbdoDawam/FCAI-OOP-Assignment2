/*
 * File: Storage.h
 * Date: 6 Oct 1973
 * Package: Storage
 * Description: Abstract base class for fixed-size hexadecimal storage units.
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>

class Storage {
protected:
    int size;
    std::vector<std::string> cells;

    std::string normalizeHex(std::string val) const;

public:
    explicit Storage(int size);
    virtual ~Storage() = 0;

    virtual std::string getCell(int index) const;
    virtual void setCell(int index, std::string val);
    virtual void clear();
    int getSize() const;
};

#endif // STORAGE_H
