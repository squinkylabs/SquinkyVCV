#pragma once

#include <memory>
#include <vector>
#include <map>

class SKeyValuePair;
using SKeyValuePairPtr = std::shared_ptr<SKeyValuePair>; 
using SKeyValueList = std::vector<SKeyValuePairPtr>;

namespace ci
{

enum class Opcode {
    NONE,
    HI_KEY,
    LO_KEY
};

enum class DiscreteValue {
    NONE
};

class Value {
public:
    int numeric;
    DiscreteValue nonNUmeric;
};

using ValuePtr = std::shared_ptr<Value>;

class KeysAndValues
{
public:
    size_t _size() const {
        return data.size();
    }
    void add(Opcode o, ValuePtr vp) {
        data[o] = vp;
    }

    ValuePtr get(Opcode o) {
        auto it = data.find(o);
        if (it == data.end()) {
            return nullptr;
        }
        return it->second;

    }
private:
    // the int key is really an Opcode
    std::map<Opcode, ValuePtr> data;

};



KeysAndValues compile(const SKeyValueList&);



}