//
// Created by Zzz0522 on 2023/12/1.
//

#include "util.h"

namespace util {
void combine(std::set<char> &lhs, std::set<char> &rhs) {
    for (auto it: rhs) {
        if (it == '@') {
            continue;
        }
        lhs.insert(it);
    }
}

bool checkDigit(std::string s) {
    for (auto c: s) {
        if (!std::isdigit(c)) {
            return 0;
        }
    }
    return 1;
}

}
