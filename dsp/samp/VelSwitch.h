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
    ISamplerPlaybackPtr mapVelToPlayer(unsigned int vel);
    void play(VoicePlayInfo&, int midiPitch, int midiVelocity) override;
    void addVelocityRange(unsigned int velRangeStart, ISamplerPlaybackPtr player);

private:

    std::map<unsigned int, ISamplerPlaybackPtr> velToPlayerMap;
};


inline void VelSwitch::addVelocityRange(unsigned int velRangeStart, ISamplerPlaybackPtr player)
{
    velToPlayerMap.insert({ velRangeStart, player });
}

inline void VelSwitch::play(VoicePlayInfo& info, int midiPitch, int midiVelocity) {
    ISamplerPlaybackPtr player = mapVelToPlayer(midiVelocity);
    player->play(info, midiPitch, midiVelocity);
}

inline ISamplerPlaybackPtr VelSwitch::mapVelToPlayer(unsigned int vel) {
    ISamplerPlaybackPtr ret;
    auto it = velToPlayerMap.lower_bound(vel);
    if (it == velToPlayerMap.end()) {
        assert(false);                  // we should always have an init entry.
        return 0;
    }
    unsigned int lb_key = it->first;
    if (lb_key > vel) {
        --it;
        ret = it->second;
    }
    else if (lb_key == vel) {
        ret = it->second;
    }
    else {
        assert(false);
    }
    return ret;
}

#if 0
inline void VelSwitch::_addIndex(unsigned int index, unsigned int value, ISamplerPlaybackPtr player)
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
#endif


