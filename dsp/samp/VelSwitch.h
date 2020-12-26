#pragma once

#include "SamplerPlayback.h"
#include <map>

/**
 * Node in the instrument tree that represents a switch on velocity
 * Implements ISamplerPlayback by lookup up velocity an a map, and branching to sub-nodes
 * under that.
 */
class VelSwitch : public ISamplerPlayback
{
public:
    unsigned int mapVelToIndex(unsigned int vel);
    void play(VoicePlayInfo&, int midiPitch, int midiVelocity) override;

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
    unsigned ret = 0;
    auto it = velIndexMap.lower_bound(vel);
    if (it == velIndexMap.end()) {
        assert(false);                  // we should always have an init entry.
        return 0;
    }
    unsigned int lb_key = it->first;
    if (lb_key > vel) {
        --it;
        ret = it->second;
    } else if (lb_key == vel) {
        ret = it->second;
    } else {
        assert(false);
    }
    return ret;;
}

inline void VelSwitch::play(VoicePlayInfo&, int midiPitch, int midiVelocity) {
    assert(false);

}
