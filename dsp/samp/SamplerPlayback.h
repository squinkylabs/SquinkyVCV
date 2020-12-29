#pragma once

#include "CompiledRegion.h"

#include <assert.h>
#include <cmath>

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

/**
 * simple one voice player. It's just a thin wrapper around
 * a structre that holds data for one voice.
 * */
class SimpleVoicePlayer : public ISamplerPlayback
{
public:
    SimpleVoicePlayer() = delete;
    SimpleVoicePlayer(CompiledRegionPtr reg, int sampleIndex, int midiPitch) {
        data->valid = true;
        data->sampleIndex = sampleIndex;

        const int semiOffset = midiPitch - reg->keycenter;
        if (semiOffset == 0) {
            data->needsTranspose = false;
            data->transposeAmt = 1;
        } else {
            const float pitchMul = float(std::pow(2, semiOffset / 12.0));
            data->needsTranspose = true;
            data->transposeAmt = pitchMul;
        }
    }
    void play(VoicePlayInfo& info, int midiPitch, int midiVelocity) override {
        info = *data;
    }

private:
    VoicePlayInfoPtr data = std::make_shared<VoicePlayInfo>();

};

/**
 * PLayer that does nothing. Hopefully will not be used (often?)
 * in the real world, but need it now to cover corner cases without
 * crashing.
 */
class NullVoicePlayer :  public ISamplerPlayback
{
public:
 void play(VoicePlayInfo& info, int midiPitch, int midiVelocity) override {
        info.valid = false;
    }
};

