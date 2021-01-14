#pragma once

#include <map>

#include "SamplerPlayback.h"

/**
 * Node in the instrument tree that represents a switch on velocity
 * Implements ISamplerPlayback by lookup up velocity an a map, and branching to sub-nodes
 * under that.
 */
class VelSwitch : public ISamplerPlayback {
public:
    ISamplerPlaybackPtr mapVelToPlayer(unsigned int vel);
    void play(VoicePlayInfo&, const VoicePlayParameter&, WaveLoader* loader, float sampleRate) override;
    void _dump(int depth) const override;
    void addVelocityRange(unsigned int velRangeStart, ISamplerPlaybackPtr player);

    VelSwitch(int line) : lineNumber(line) {}

private:
    std::map<unsigned int, ISamplerPlaybackPtr> velToPlayerMap;
    bool addedOne = false;
    const int lineNumber;
};

inline void VelSwitch::addVelocityRange(unsigned int velRangeStart, ISamplerPlaybackPtr player) {
    // printf("in addVelocityRange(%d)\n", velRangeStart);
    velToPlayerMap.insert({velRangeStart, player});
    if (velRangeStart == 1) {
        addedOne = true;
    }
}

inline void VelSwitch::play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    ISamplerPlaybackPtr player = mapVelToPlayer(params.midiVelocity);
    player->play(info, params, loader, sampleRate);
}

inline ISamplerPlaybackPtr VelSwitch::mapVelToPlayer(unsigned int vel) {
    // printf("in mapVelToPlayer(%d)\n", vel);

    assert(vel > 0);
    assert(addedOne);
    ISamplerPlaybackPtr ret;
    auto it = velToPlayerMap.lower_bound(vel);
    if (it == velToPlayerMap.end()) {
        if (velToPlayerMap.empty()) {
            assert(false);
            return nullptr;
        }

        it--;
        // printf("in mapVelToPlayer vel=%d, went off end of map prev index=%d\n", vel, it->first); fflush(stdout);
        return it->second;
    }
    unsigned int lb_key = it->first;
    if (lb_key > vel) {
        --it;
        ret = it->second;
    } else if (lb_key == vel) {
        ret = it->second;
    } else {
        // printf("in mapVelToPlayer vel=%d, lb_key =%d\n", vel, lb_key); fflush(stdout);
        assert(false);
    }
    return ret;
}

inline void VelSwitch::_dump(int depth) const {
    indent(depth);
    printf("begin vel switch ent=%d this=%p\n", int(velToPlayerMap.size()), this);
    for (auto entry : velToPlayerMap) {
        indent(depth + 1);
        printf("entry at vel %d:\n", entry.first);
        entry.second->_dump(depth + 1);
    }

    indent(depth);
    printf("end vel switch %p\n", this);
}

using VelSwitchPtr = std::shared_ptr<VelSwitch>;
