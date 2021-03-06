

#include "SamplerPlayback.h"

#include "SqLog.h"
#include "WaveLoader.h"

VoicePlayInfo::VoicePlayInfo(CompiledRegionPtr region, int midiPitch, int sampleIndex) {
    this->valid = true;
    this->sampleIndex = sampleIndex;

    const int semiOffset = midiPitch - region->keycenter;
    if (semiOffset == 0) {
        this->needsTranspose = false;
#ifdef _SAMPFM
        this->transposeV = 0;
#else
        this->transposeAmt = 1;
#endif
    } else {
        this->needsTranspose = true;
#ifdef _SAMPFM
        assert(false);
#else
        const float pitchMul = float(std::pow(2, semiOffset / 12.0));
        this->transposeAmt = pitchMul;
#endif
    }
}

void SimpleVoicePlayer::play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    cachedInfoToPlayInfo(info, params, data);
    if (loader) {
        // do we need to adapt to changed sample rate?
        unsigned int waveSampleRate = loader->getInfo(info.sampleIndex)->sampleRate;
        if (!AudioMath::closeTo(sampleRate, waveSampleRate, 1)) {
            info.needsTranspose = true;
#ifdef _SAMPFM
            // this ratio was alwyas backward. fixed now
            const float transposeRatio = float(waveSampleRate) / sampleRate;
            const float srShiftV = std::log2(transposeRatio);
            info.transposeV = data.transposeV + srShiftV;
#else
            info.transposeAmt = data.transposeAmt * sampleRate / float(waveSampleRate);
#endif
        }
    }
}

#if 0
void RandomVoicePlayer::_dump(int depth) const {
    indent(depth);
    printf("Random Voice Player (tbd)\n");
}

void RandomVoicePlayer::play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    assert(finalized);
    if (entries.empty()) {
        SQWARN("RandomPlayer has no entries");
        info.valid = false;
        return;
    }

    int index = rand.get();
    if (index >= int(entries.size())) {
        SQWARN("RandomPlayer index out of bounds");
        index = int(entries.size()) - 1;
    }

    cachedInfoToPlayInfo(info, params, *entries[index]);
    assert(info.valid);
}

#define _NEWW

#ifdef _NEWW
void RandomVoicePlayer::addEntry(CompiledRegionPtr region, int sampleIndex, int midiPitch) {
    TempHolder holder;
    holder.info = std::make_shared<CachedSamplerPlaybackInfo>(region, midiPitch, sampleIndex);
    holder.hirand = region->hirand;
    tempEntries.push_back(holder);
}

void RandomVoicePlayer::finalize() {
    assert(!finalized);
    finalized = true;
    if (tempEntries.empty()) {
        SQWARN("random group with no entries");
        return;
    }

    // first sort temp entries by hirand
    std::sort(tempEntries.begin(), tempEntries.end(), [](const TempHolder& a, const TempHolder& b) -> bool {
        bool less = false;
        if (a.hirand < b.hirand) {
            less = true;
        }
        return less;
    });

    // make it all valid
    float largest = tempEntries.back().hirand;
    if (largest > 1) {
        SQWARN("correct error in probabilities %f", largest);

        // If the probabilities are whack, just make them all the same.
        // This happens if there is a typo in the SFZ file.

        const float avg = 1.f / tempEntries.size();
        float nextValue = avg;
        // for (TempHolder& ent : tempEntries) {
        for (size_t i = 0; i < tempEntries.size(); ++i) {
            tempEntries[i].hirand = nextValue;
            nextValue += avg;
        }
    }

    // then add them
    for (auto ent : tempEntries) {
        entries.push_back(ent.info);
        rand.addRange(ent.hirand);
    }
}
#else

void RandomVoicePlayer::finalize() {
    finalized = true;
}

void RandomVoicePlayer::addEntry(CompiledRegionPtr region, int sampleIndex, int midiPitch) {
    int index = int(entries.size());
    if (index == 0) {
        assert(region->lorand == 0);
        ++index;
    }
    //   VoicePlayInfoPtr info = std::make_shared<VoicePlayInfo>(region, midiPitch, sampleIndex);
    //    std::vector<CachedSamplerPlaybackInfoPtr> entries;
    CachedSamplerPlaybackInfoPtr info = std::make_shared<CachedSamplerPlaybackInfo>(region, midiPitch, sampleIndex);
    entries.push_back(info);
    rand.addRange(region->hirand);
    printf("rand add range entries=%d hirane=%f\n", (int)entries.size(), region->hirand);
}
#endif

RoundRobinVoicePlayer::RRPlayInfo::RRPlayInfo(const CachedSamplerPlaybackInfo& info) : CachedSamplerPlaybackInfo(info) {
}

void RoundRobinVoicePlayer::_dump(int depth) const {
    indent(depth);
    printf("Round Robin Voice Payer (tbd)");
}

void RoundRobinVoicePlayer::play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    if (currentEntry >= numEntries) {
        currentEntry = 0;
    }
    cachedInfoToPlayInfo(info, params, *entries[currentEntry]);
    ++currentEntry;
}

//   RRPlayInfo(const CachedSamplerPlaybackInfo&);
void RoundRobinVoicePlayer::addEntry(CompiledRegionPtr region, int sampleIndex, int midiPitch) {
    CachedSamplerPlaybackInfoPtr info = std::make_shared<CachedSamplerPlaybackInfo>(region, midiPitch, sampleIndex);
    RRPlayInfoPtr rr_info = std::make_shared<RRPlayInfo>(*info);
    rr_info->seq_position = region->sequencePosition;
    entries.push_back(rr_info);
    numEntries = int(entries.size());
}

void RoundRobinVoicePlayer::finalize() {
    std::sort(entries.begin(), entries.end(), [](const RRPlayInfoPtr a, const RRPlayInfoPtr b) -> bool {
        bool less = false;
        if (a->seq_position < b->seq_position) {
            less = true;
        }
        return less;
    });
    currentEntry = int(entries.size()) + 1;
}
#endif
