#pragma once
#include <map>

class VelSwitch
{
public:
    unsigned int mapVelToIndex(unsigned int vel);

    void _addIndex(unsigned int index, unsigned int value);
private:
    // velocity(range) to index
    std::map<unsigned int, unsigned int> velIndexMap;
};

inline void VelSwitch::_addIndex(unsigned int index, unsigned int value)
{

    velIndexMap.insert({value, index});
}

inline unsigned  VelSwitch::mapVelToIndex(unsigned  vel)
{

    auto it = velIndexMap.lower_bound(vel);
    if (it == velIndexMap.end()) {
        assert(false);                  // we should always have an init entry.
        return 0;
    }
    unsigned int index = it->second;
    return index;
}