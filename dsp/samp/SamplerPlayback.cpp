
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
