#pragma once

#include <assert.h>

#include <cmath>

#include "CompiledRegion.h"
#include "RandomRange.h"

class WaveLoader;

#define _SAMPFM

/**
 * When a patch is asked to "play", it serves up one of these.
 * So this is the "output" of play.
 */
class VoicePlayInfo {
public:
    VoicePlayInfo() = default;
    VoicePlayInfo(CompiledRegionPtr region, int midiPitch, int sampleIndex);
    bool valid = false;
    int sampleIndex = 0;
    bool needsTranspose = false;
#ifdef _SAMPFM
    float transposeV = 0;       // 1V/Oct, synth standard
#else
    float transposeAmt = 1;
#endif
    float gain = 1;  // assume full volume
    float ampeg_release = .001f;

    bool canPlay() const {
        return valid && (sampleIndex > 0);
    }

    CompiledRegion::LoopData loopData;
};
using VoicePlayInfoPtr = std::shared_ptr<VoicePlayInfo>;

/**
 * This is info send down to play.
 * So it is the "input" to play.
 */
class VoicePlayParameter {
public:
    int midiPitch = 0;
    int midiVelocity = 0;
};

class ISamplerPlayback {
public:
    virtual ~ISamplerPlayback() = default;

    /**
     * @param info is where the playback info is returned.
     * @param params are not play parameters.
     * @param loader is all the wave files. may be null for some tests.
     * @param sampleRate is the current sample rate. Ignored if loader is nullptr
     * @returns true is note caused a key-switch
     */
    virtual bool play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) = 0;
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
 * Data extracted from patch required to play one note 
 */
class CachedSamplerPlaybackInfo {
public:
    CachedSamplerPlaybackInfo() = delete;
    CachedSamplerPlaybackInfo(CompiledRegionPtr reg, int midiPitch, int sampleIndex) : sampleIndex(sampleIndex) {
        const int semiOffset = midiPitch - reg->keycenter;
        if (semiOffset == 0) {
            needsTranspose = false;
#ifdef _SAMPFM
            transposeV = 0;
#else
            transposeAmt = 1;
#endif
        } else {
            needsTranspose = true;
#ifdef _SAMPFM
            assert(false);
#else
            const float pitchMul = float(std::pow(2, semiOffset / 12.0));
            transposeAmt = pitchMul;
#endif
        }
        amp_veltrack = reg->amp_veltrack;
        ampeg_release = reg->ampeg_release;
    }

    // properties that get served up unchanged
    bool needsTranspose = false;
#ifdef _SAMPFM
    float transposeV = 0;
#else
    a b c
    float transposeAmt = 1;
#endif
    const int sampleIndex;
    float ampeg_release = .001f;

    // properties that participate in calculations
    float amp_veltrack = 100;
};

using CachedSamplerPlaybackInfoPtr = std::shared_ptr<CachedSamplerPlaybackInfo>;

// TODO: move tis into a class??
inline void cachedInfoToPlayInfo(VoicePlayInfo& playInfo, const VoicePlayParameter& params, const CachedSamplerPlaybackInfo& cachedInfo) {
    assert(params.midiVelocity > 0 && params.midiVelocity <= 127);
    playInfo.sampleIndex = cachedInfo.sampleIndex;
    playInfo.needsTranspose = cachedInfo.needsTranspose;
#ifdef _SAMPFM
    playInfo.transposeV = cachedInfo.transposeV;
#else
    playInfo.transposeAmt = cachedInfo.transposeAmt;
#endif
    playInfo.ampeg_release = cachedInfo.ampeg_release;
    playInfo.valid = true;

    // compute gain
    {
        // first do the veltrack adjustment to the raw velocity
        const float v = float(params.midiVelocity);
        const float t = cachedInfo.amp_veltrack;
        const float x = (v * t / 100.f) + (100.f - t) * (127.f / 100.f);

        // then taper it
        auto temp = float(x) / 127.f;
        temp *= temp;
        playInfo.gain = temp;

        // printf("doing vel comp veloc=%d, track=%f x=%f, gain=%f\n", params.midiVelocity, t, x, playInfo.gain);
    }
}

class SimpleVoicePlayer : public ISamplerPlayback {
public:
    SimpleVoicePlayer() = delete;
    SimpleVoicePlayer(CompiledRegionPtr reg, int midiPitch, int sampleIndex) : data(reg, midiPitch, sampleIndex),
                                                                               lineNumber(reg->lineNumber) {
        assert(sampleIndex > 0);
    }
    bool play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) override;
    void _dump(int depth) const override {
        indent(depth);
        printf("simple voice player si=%d\n", data.sampleIndex);
    }

private:
    CachedSamplerPlaybackInfo data;
    const int lineNumber;  // in the source file
};

/**
 * PLayer that does nothing. Hopefully will not be used (often?)
 * in the real world, but need it now to cover corner cases without
 * crashing.
 */
#if 0   // do wo use this?
class NullVoicePlayer : public ISamplerPlayback {
public:
    void play(VoicePlayInfo& info, const VoicePlayParameter&, WaveLoader* loader, float sampleRate) override {
        info.valid = false;
    }
    void _dump(int depth) const override {
        indent(depth);
        printf("NullVoicePlayer %p\n", this);
    }
};
#endif
