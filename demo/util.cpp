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
        if (!isdigit(c)) {
            return 0;
        }
    }
    return 1;
}

std::string combineDot(Item &t)
{
    std::string res = "";
    std::string lhs = t.second;
    int pos = t.idx;
    if (lhs.empty()) {
        res = ".";
    } else {
        res = lhs.substr(0, pos) + "." + lhs.substr(pos);
    }
    return res;
}

}
