#ifndef COMPARE_H
#define COMPARE_H
#include <string>
#include <utility>
#include "utils/kvEntry.h"

struct CompareString
{
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        if (lhs.length() != rhs.length())
            return lhs.length() < rhs.length();
        return lhs < rhs;
    }
};

struct ComparePair
{
    bool operator()(const KvEntry &lhs,
                    const KvEntry &rhs) const
    {
        CompareString cmp;
        if (cmp(lhs.key, rhs.key))
            return true;
        if (cmp(rhs.key, lhs.key))
            return false; // lhs.key == rhs.key
        return cmp(lhs.value, rhs.value);
    }
};
#endif