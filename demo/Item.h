#ifndef ITEM_H
#define ITEM_H

#include <string>

struct Item {
    char first;
    std::string second;
    int idx;

    Item() = default;
    Item(char _first = 0, std::string _second = "", int _idx = 0) : first(_first), second(_second), idx(_idx) {}

    bool operator < (const Item &rhs) const {
        if (first != rhs.first) {
            return first < rhs.first;
        } else if (second != rhs.second) {
            return second < rhs.second;
        } else {
            return idx < rhs.idx;
        }
    }

    bool operator==(const Item &rhs) const {
        return first == rhs.first && second == rhs.second && idx == rhs.idx;
    }

};

#endif // ITEM_H
