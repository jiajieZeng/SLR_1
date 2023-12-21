//
// Created by Zzz0522 on 2023/12/1.
//

#ifndef DEMO_UTIL_H
#define DEMO_UTIL_H

#include "Item.h"

#include <set>
#include <string>

namespace util {
void combine(std::set<char> &lhs, std::set<char> &rhs);
bool checkDigit(std::string s);
std::string combineDot(Item &t);
std::string combineFollow(std::set<char> &st);
}

#endif //DEMO_UTIL_H
