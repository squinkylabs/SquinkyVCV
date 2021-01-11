

#include "SamplerPlayback.h"
#include "SqLog.h"

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

void SimpleVoicePlayer::play(VoicePlayInfo& info, const VoicePlayParameter& params) {
    cachedInfoToPlayInfo(info, params, data);
}

void RandomVoicePlayer::_dump(int depth) const {
    indent(depth);
    printf("Random Voice Player (tbd)\n");
}

void RandomVoicePlayer::play(VoicePlayInfo& info, const VoicePlayParameter& params) {
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
}

RoundRobinVoicePlayer::RRPlayInfo::RRPlayInfo(const CachedSamplerPlaybackInfo& info) : CachedSamplerPlaybackInfo(info) {
}

void RoundRobinVoicePlayer::_dump(int depth) const {
    indent(depth);
    printf("Round Robin Voice Payer (tbd)");
}

void RoundRobinVoicePlayer::play(VoicePlayInfo& info, const VoicePlayParameter& params) {
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
    rr_info->seq_position = region->seq_position;
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
}
