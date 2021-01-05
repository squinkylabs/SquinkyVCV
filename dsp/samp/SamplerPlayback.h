#pragma once

#include <assert.h>

#include <cmath>

#include "RandomRange.h"
#include "CompiledRegion.h"

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
    float transposeAmt = 1;
    float gain = 1;             // assume full volume

    bool canPlay() const {
        return valid && (sampleIndex > 0);
    }
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
    // TODO: should this return VoicePlayInfoPtr??
    virtual void play(VoicePlayInfo&, const VoicePlayParameter&) = 0;
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
            transposeAmt = 1;
        } else {
            const float pitchMul = float(std::pow(2, semiOffset / 12.0));
            needsTranspose = true;
            transposeAmt = pitchMul;
        }
    }

    bool needsTranspose = false;
    float transposeAmt = 1;
    const int sampleIndex;
};

using CachedSamplerPlaybackInfoPtr = std::shared_ptr<CachedSamplerPlaybackInfo>;

inline void cachedInfoToPlayInfo(VoicePlayInfo& playInfo, const VoicePlayParameter& params, const CachedSamplerPlaybackInfo& cachedInfo) {
    assert(params.midiVelocity > 0 && params.midiVelocity <= 127);
    playInfo.sampleIndex = cachedInfo.sampleIndex;
    playInfo.needsTranspose = cachedInfo.needsTranspose;
    playInfo.transposeAmt = cachedInfo.transposeAmt;
    playInfo.valid = true;
    auto temp = float(params.midiVelocity) / 127.f;
    temp *= temp;
    playInfo.gain = temp;
}

class SimpleVoicePlayer : public ISamplerPlayback {
public:
    SimpleVoicePlayer() = delete;
    SimpleVoicePlayer(CompiledRegionPtr reg, int midiPitch, int sampleIndex) :
        data(reg, sampleIndex, midiPitch),
        lineNumber(reg->lineNumber) {

    }
    void play(VoicePlayInfo& info, const VoicePlayParameter& params) override;
    void _dump(int depth) const override {
        indent(depth);
        printf("simple voice player si=%d\n", data.sampleIndex);
    }

private:
    CachedSamplerPlaybackInfo data;
    const int lineNumber;  // in the source file
};

#if 0
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
    void play(VoicePlayInfo& info, const VoicePlayParameter& params) override {
        assert(params.midiVelocity > 0 && params.midiVelocity <= 127);
        info = *data;
        assert(false); // what about amp vel?
    }
    void _dump(int depth) const override {
        indent(depth);
        printf("simple voice player si=%d\n", data->sampleIndex);
    }

private:
    VoicePlayInfoPtr data = std::make_shared<VoicePlayInfo>();
    const int lineNumber;  // in the source file
};
#endif


/**
 * PLayer that does nothing. Hopefully will not be used (often?)
 * in the real world, but need it now to cover corner cases without
 * crashing.
 */
class NullVoicePlayer : public ISamplerPlayback {
public:
    void play(VoicePlayInfo& info, const VoicePlayParameter&) override {
        info.valid = false;
    }
    void _dump(int depth) const override {
        indent(depth);
        printf("NullVoicePlayer %p\n", this);
    }
};


class RandomVoicePlayer : public ISamplerPlayback {
public:
    RandomVoicePlayer() : rand(0) {}
    void play(VoicePlayInfo& info, const VoicePlayParameter&) override;
    void _dump(int depth) const override;
    void addEntry(CompiledRegionPtr region, int sampleIndex, int midiPitch);
private:
    std::vector<CachedSamplerPlaybackInfoPtr> entries;
    RandomRange<float> rand;
};

using RandomVoicePlayerPtr = std::shared_ptr<RandomVoicePlayer>;

class RoundRobinVoicePlayer : public ISamplerPlayback {
public:
    // let's add seq_position so we can play in the right order
    class RRPlayInfo : public CachedSamplerPlaybackInfo {
    public:
        RRPlayInfo(const CachedSamplerPlaybackInfo&);
        int seq_position = 0;
    };
    using RRPlayInfoPtr = std::shared_ptr<RRPlayInfo>;
    void play(VoicePlayInfo& info, const VoicePlayParameter&) override;
    void _dump(int depth) const override;
    void addEntry(CompiledRegionPtr region, int sampleIndex, int midiPitch);
    void finalize();
private:
    std::vector<RRPlayInfoPtr> entries;
    int currentEntry = 0;
    int numEntries = 0;
   
};

using RoundRobinVoicePlayerPtr = std::shared_ptr<RoundRobinVoicePlayer>;