#pragma once

#include "CompiledRegion.h"
/**
 * When a patch is asked to "play", it serves up one of these.
 */
class VoicePlayInfo {
public:
    bool valid = false;
    int sampleIndex = 0;
    bool needsTranspose = false;
    float transposeAmt = 1;

    bool canPlay() const {
        return valid && (sampleIndex > 0);
    }
};
using VoicePlayInfoPtr = std::shared_ptr<VoicePlayInfo>;


class ISamplerPlayback {
public:
    virtual ~ISamplerPlayback() = default;
    // TODO: should this return VoicePlayInfoPtr??
    virtual void play(VoicePlayInfo&, int midiPitch, int midiVelocity) = 0;
};

using ISamplerPlaybackPtr = std::shared_ptr<ISamplerPlayback>;

class SimpleVoicePlayer : public ISamplerPlayback
{
public:
    SimpleVoicePlayer(CompiledRegionPtr reg, int sampleIndex) {
        data->valid = true;
        data->sampleIndex = sampleIndex;
        printf("SimpleVoicePlayer can't figure out transpose yet\n");
    }
    void play(VoicePlayInfo& info, int midiPitch, int midiVelocity) override {
        info = *data;
    }

private:
    VoicePlayInfoPtr data = std::make_shared<VoicePlayInfo>();

};

