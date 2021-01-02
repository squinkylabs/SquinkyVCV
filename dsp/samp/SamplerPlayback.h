#pragma once

#include <assert.h>

#include <cmath>

#include "CompiledRegion.h"

/**
 * When a patch is asked to "play", it serves up one of these.
 */
class VoicePlayInfo {
public:
    VoicePlayInfo() = default;
    VoicePlayInfo(CompiledRegionPtr region, int midiPitch, int sampleIndex);
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
    virtual void _dump(int depth) const = 0;

protected:
    static void indent(int depth) {
        for (int i = 0; i < depth; ++i) {
            printf("  ");
        }
    }
};

using ISamplerPlaybackPtr = std::shared_ptr<ISamplerPlayback>;

/**
 * simple one voice player. It's just a thin wrapper around
 * a structre that holds data for one voice.
 * */


class SimpleVoicePlayer : public ISamplerPlayback {
public:
    SimpleVoicePlayer() = delete;
    SimpleVoicePlayer(CompiledRegionPtr reg, int sampleIndex, int midiPitch) : lineNumber(reg->lineNumber) {
        data->valid = true;
        data->sampleIndex = sampleIndex;

        printf("recode 58 the new way\n");
        //assert(false);      // re code this the new easy way
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
        assert(midiVelocity > 0 && midiVelocity <= 127);
        info = *data;
    }
    void _dump(int depth) const override {
        indent(depth);
        printf("simple voice player si=%d\n", data->sampleIndex);
    }

private:
    VoicePlayInfoPtr data = std::make_shared<VoicePlayInfo>();
    const int lineNumber;  // in the source file
};


/**
 * PLayer that does nothing. Hopefully will not be used (often?)
 * in the real world, but need it now to cover corner cases without
 * crashing.
 */
class NullVoicePlayer : public ISamplerPlayback {
public:
    void play(VoicePlayInfo& info, int midiPitch, int midiVelocity) override {
        info.valid = false;
    }
    void _dump(int depth) const override {
        indent(depth);
        printf("NullVoicePlayer %p\n", this);
    }
};

class MultiVoicePlayer  {
protected:
    std::vector<VoicePlayInfoPtr> variations;

};

class RandomVoicePlayer : public ISamplerPlayback, MultiVoicePlayer {
public:
    void play(VoicePlayInfo& info, int midiPitch, int midiVelocity) override;
    void _dump(int depth);
private:
   
};

class RoundRobbinVoicePlayer : public ISamplerPlayback, MultiVoicePlayer {
public:
    void play(VoicePlayInfo& info, int midiPitch, int midiVelocity) override;
    void _dump(int depth);
private:
   
};