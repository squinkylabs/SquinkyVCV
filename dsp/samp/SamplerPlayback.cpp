
#include "SamplerPlayback.h"

VoicePlayInfo::VoicePlayInfo(CompiledRegionPtr region, int midiPitch, int sampleIndex) {
    this->valid = true;
    this->sampleIndex = sampleIndex;

    const int semiOffset = midiPitch - region->keycenter;
    if (semiOffset == 0) {
        this->needsTranspose = false;
        this->transposeAmt = 1;
    } else {
        const float pitchMul = float(std::pow(2, semiOffset / 12.0));
        this->needsTranspose = true;
        this->transposeAmt = pitchMul;
    }
}

void RandomVoicePlayer::_dump(int depth) const {
    indent(depth);
    printf("Random Voice Payer (tbd)");
}

void RandomVoicePlayer::play(VoicePlayInfo& info, int midiPitch, int midiVelocity) {
    const int index = rand.get();
#if 0
    const float r = randomFunc();
    
    auto it = probabilityRanges.lower_bound(r);
    int index = 0;

    // TODO: wrap up this logic in its own class
    if (it == probabilityRanges.end()) {
        if (probabilityRanges.empty()) {
            assert(false);
            info.valid = false;
            return;
        }

        it--;
        // printf("in mapVelToPlayer vel=%d, went off end of map prev index=%d\n", vel, it->first); fflush(stdout);
        index = it->second;
    }
    float lb_key = it->first;
    if (lb_key > r) {
        --it;
        assert(it != probabilityRanges.end());
        index = it->second;
    } else if (lb_key == r) {
        index = it->second;
    } else {
        // printf("in mapVelToPlayer vel=%d, lb_key =%d\n", vel, lb_key); fflush(stdout);
        assert(false);
    }
#endif

    assert(index < entries.size());
    info = *entries[index];
    printf("in play ran index=%d\n", index);
    assert(info.valid);
}

//   SimpleVoicePlayer(CompiledRegionPtr reg, int sampleIndex, int midiPitch
void RandomVoicePlayer::addEntry(CompiledRegionPtr region, int sampleIndex, int midiPitch) {
    int index = int(entries.size());
    if (index == 0) {
        assert(region->lorand == 0);
       // probabilityRanges.insert({0.f, 0});
        ++index;
    }
    VoicePlayInfoPtr info = std::make_shared<VoicePlayInfo>(region, midiPitch, sampleIndex);
    entries.push_back(info);
   // probabilityRanges.insert({region->hirand, index});
    rand.addRange(region->hirand);
    printf("added prob entry probh=%f si=%d, pitch=%d\n", region->hirand, sampleIndex, midiPitch );
    //std::vector<VoicePlayInfoPtr> entries;
}

void RoundRobinVoicePlayer::_dump(int depth) const {
    indent(depth);
    printf("Round Robin Voice Payer (tbd)");
}

void RoundRobinVoicePlayer::play(VoicePlayInfo& info, int midiPitch, int midiVelocity) {
    assert(false);
}
#if 0
class RandomVoicePlayer : public ISamplerPlayback {
public:
    class Entry {
        VoicePlayInfo info;
        float probabilty = 0;
    };
    using EntryPtr = std::shared_ptr<Entry>;
    void play(VoicePlayInfo& info, int midiPitch, int midiVelocity) override;
    void _dump(int depth);
    void addEntry(EntryPtr);
private:
    std::vector<EntryPtr> entries;
   
};
#endif
