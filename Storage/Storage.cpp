#include "Storage.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;


class Storage{
    protected:
        int size;
        vector<string> cells;

        string normalizeHexValue(string val) const { 
            transform(val.begin(), val.end(), val.begin(), ::toupper);

            if (val.length() < 2) {
                val = "0" + val;
            }

            return val;
        }

    public:
        Storage(int s) : size(s) {
            cells.resize(size);
        }

        virtual string getCell(int index) const {
            if (index >= 0 && index < size) {
                return cells[index];
            }
            cout << "Index out of bounds" << endl;
            return "";
        }

        virtual void setCell(int index, string val) {
            if (index >= 0 && index < size) {
                cells[index] = normalizeHexValue(val);
            }
            else {
                cout << "Index out of bounds" << endl;
            }
        }

        virtual void clear() {
            fill(cells.begin(), cells.end(), "00");
        }

        int getSize() const {
            return size;
        }

        virtual ~Storage() {
            cout << "Storage destroyed" << endl;
        }
};