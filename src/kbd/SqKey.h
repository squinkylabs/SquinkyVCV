#pragma once

#include <memory>
#include <map>

class SqKey;
struct json_t;
using SqKeyPtr = std::shared_ptr<SqKey>;

class SqKey
{
public:
    SqKey(int key, bool ctrl, bool shift, bool alt) : key(key), ctrl(ctrl), shift(shift), alt(alt) {}
    static SqKeyPtr parse(json_t* binding);
    bool operator < (const SqKey&) const;

    const int key = 0;
    const bool ctrl = false;
    const bool shift = false;
    const bool alt = false;
    static int parseKey(const std::string&);
private:
   
    static void initMap();

    static std::map<std::string, int> keyString2KeyCode;
};