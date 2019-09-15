#pragma once

#include <memory>

class SqKey;
struct json_t;
using SqKeyPtr = std::shared_ptr<SqKey>;

class SqKey
{
public:
    static SqKeyPtr parse(json_t* binding);
    bool operator < (const SqKey&) const;
private:
    SqKey(int key, bool ctrl, bool shift) : key(key), ctrl(ctrl), shift(shift) {}


    const int key = 0;
    const bool ctrl = false;
    const bool shift = false;
};