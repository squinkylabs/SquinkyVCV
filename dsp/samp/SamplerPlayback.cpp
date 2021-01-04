
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
    printf("Random Voice Player (tbd)\n");
}

void RandomVoicePlayer::play(VoicePlayInfo& info, int midiPitch, int midiVelocity) {
    const int index = rand.get();
    assert(index < entries.size());
    info = *entries[index];
    printf("in play ran index=%d\n", index);
    assert(info.valid);
}


void RandomVoicePlayer::addEntry(CompiledRegionPtr region, int sampleIndex, int midiPitch) {
    int index = int(entries.size());
    if (index == 0) {
        assert(region->lorand == 0);
        ++index;
    }
    VoicePlayInfoPtr info = std::make_shared<VoicePlayInfo>(region, midiPitch, sampleIndex);
    entries.push_back(info);
    rand.addRange(region->hirand);
}

void RoundRobinVoicePlayer::_dump(int depth) const {
    indent(depth);
    printf("Round Robin Voice Payer (tbd)");
}

void RoundRobinVoicePlayer::play(VoicePlayInfo& info, int midiPitch, int midiVelocity) {
    assert(false);
}

