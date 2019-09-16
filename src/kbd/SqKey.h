#pragma once

#include <memory>
#include <map>

class SqKey;
struct json_t;
using SqKeyPtr = std::shared_ptr<SqKey>;

class SqKey
{
public:
    SqKey(int key, bool ctrl, bool shift) : key(key), ctrl(ctrl), shift(shift) {}
    static SqKeyPtr parse(json_t* binding);
    bool operator < (const SqKey&) const;

    const int key = 0;
    const bool ctrl = false;
    const bool shift = false;
private:
    static int parseKey(const std::string&);
    static void initMap();

    static std::map<std::string, int> keyString2KeyCode;
};