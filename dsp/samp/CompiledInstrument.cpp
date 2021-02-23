
#include "CompiledInstrument.h"

#include <assert.h>

#include <algorithm>
#include <cmath>
#include <set>
#include <string>

#include "CompiledRegion.h"
#include "InstrumentInfo.h"
#include "SInstrument.h"
#include "SParse.h"
#include "SamplerPlayback.h"
#include "SqLog.h"
#include "WaveLoader.h"

using Opcode = SamplerSchema::Opcode;
using OpcodeType = SamplerSchema::OpcodeType;
using DiscreteValue = SamplerSchema::DiscreteValue;
using ValuePtr = SamplerSchema::ValuePtr;
using Value = SamplerSchema::Value;

//#define _LOG
//#define _LOGOV

bool CompiledInstrument::compile(const SInstrumentPtr in) {
    assert(in->wasExpanded);
    bool ret = regionPool.buildCompiledTree(in);
    if (!ret) {
        return false;
    }

    addSampleIndexes();
    deriveInfo();
    assert(info);
    return true;
}

void CompiledInstrument::addSampleIndexes() {
    regionPool.visitRegions([this](CompiledRegion* region) {
        int index = this->addSampleFile(region->sampleFile);
        assert(0 == region->sampleIndex);
        region->sampleIndex = index;
    });
}

void CompiledInstrument::deriveInfo() {
    info = std::make_shared<InstrumentInfo>();
    regionPool.visitRegions([this](CompiledRegion* region) {
        if (region->sw_lolast >= 0) {
            std::string label = region->sw_label.empty() ? "(untitled)" : region->sw_label;
            int low = region->sw_lolast;
            int hi = region->sw_hilast;

            InstrumentInfo::PitchRange range = std::pair<int, int>(low, hi);
            auto iter = info->keyswitchData.find(label);

            if (iter != info->keyswitchData.end()) {
                InstrumentInfo::PitchRange existingRange = iter->second;
                range.first = std::min(range.first, existingRange.first);
                range.second = std::max(range.second, existingRange.second);
                iter->second = range;
            } else {
                // it's not there already. insert
                info->keyswitchData.insert(std::pair<std::string, InstrumentInfo::PitchRange>(label, range));
            }
        }

        info->minPitch = (info->minPitch < 0) ? region->lokey : std::min(info->minPitch, region->lokey);
        info->maxPitch = std::max(info->maxPitch, region->hikey);

        if (region->sw_default >= 0) {
            info->defaultKeySwitch = region->sw_default;
        }
    });
}

/** build up the tree using the original algorithm that worked for small piano
 * we don't need structure here, to flattened region list is find
 */

class RegionBin {
public:
    int loVal = -1;
    int hiVal = -1;
    std::vector<CompiledRegionPtr> regions;
};

static void dumpRegions(const std::vector<CompiledRegionPtr>& inputRegions) {
    int x = 0;
    for (auto reg : inputRegions) {
        printf("    reg[%d] #%d pitch=%d,%d vel=%d,%d\n", x, reg->lineNumber, reg->lokey, reg->hikey, reg->lovel, reg->hivel);
        ++x;
    }
}

void CompiledInstrument::_dump(int depth) const {
    regionPool._dump(depth);
}

int CompiledInstrument::addSampleFile(const std::string& s) {
    int ret = 0;
    auto it = relativeFilePaths.find(s);
    if (it != relativeFilePaths.end()) {
        ret = it->second;
    } else {
        relativeFilePaths.insert({s, nextIndex});
        ret = nextIndex++;
    }
    return ret;
}

CompiledInstrumentPtr CompiledInstrument::CompiledInstrument::make(SamplerErrorContext& err, SInstrumentPtr inst) {
    assert(!inst->wasExpanded);
    expandAllKV(err, inst);
    CompiledInstrumentPtr instOut = std::make_shared<CompiledInstrument>();
    const bool result = instOut->compile(inst);
    return result ? instOut : nullptr;
}

void CompiledInstrument::getGain(VoicePlayInfo& info, int midiVelocity, float regionVeltrack) {
    const float v = float(midiVelocity);
    const float t = regionVeltrack;
    const float x = (v * t / 100.f) + (100.f - t) * (127.f / 100.f);

    // then taper it
    auto temp = float(x) / 127.f;
    temp *= temp;
    info.gain = temp;
}

void CompiledInstrument::getPlayPitch(VoicePlayInfo& info, int midiPitch, int regionKeyCenter, WaveLoader* loader, float sampleRate) {
    // first base pitch
    const int semiOffset = midiPitch - regionKeyCenter;
    if (semiOffset == 0) {
        info.needsTranspose = false;
        info.transposeAmt = 1;
    } else {
        const float pitchMul = float(std::pow(2, semiOffset / 12.0));
        info.needsTranspose = true;
        info.transposeAmt = pitchMul;
    }

    // then sample rate correction
    if (loader) {
        // do we need to adapt to changed sample rate?
        unsigned int waveSampleRate = loader->getInfo(info.sampleIndex)->sampleRate;
        if (!AudioMath::closeTo(sampleRate, waveSampleRate, 1)) {
            info.needsTranspose = true;
            info.transposeAmt *= sampleRate / float(waveSampleRate);
        }
    }
}

void CompiledInstrument::play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    if (testMode != Tests::None) {
        return playTestMode(info, params, loader, sampleRate);
    }
    info.valid = false;
    float r = rand();
    const CompiledRegion* region = regionPool.play(params, r);
    if (region) {
        info.sampleIndex = region->sampleIndex;
        info.valid = true;
        info.ampeg_release = region->ampeg_release;
        getPlayPitch(info, params.midiPitch, region->keycenter, loader, sampleRate);
        getGain(info, params.midiVelocity, region->amp_veltrack);
    }
}

void CompiledInstrument::playTestMode(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    assert(testMode == Tests::MiddleC);

    assert(params.midiPitch == 60);  // probably a mistake it a tests isn't using this pitch
    info.sampleIndex = 1;
    info.valid = true;
    info.needsTranspose = false;
    info.transposeAmt = 1;
    info.ampeg_release = .6f;        // standard default for me.
}

void CompiledInstrument::setWaves(WaveLoaderPtr loader, const FilePath& rootPath) {
    std::vector<std::string> tempPaths;
    assert(!rootPath.empty());

    auto num = relativeFilePaths.size();
    tempPaths.resize(num);
    // index is 1..
    for (auto pathEntry : relativeFilePaths) {
        std::string path = pathEntry.first;

        int waveIndex = pathEntry.second;
        //printf("in setWaves, entry has %s index = %d\n", path.c_str(), waveIndex);
        assert(waveIndex > 0);
        assert(!path.empty());
        tempPaths[waveIndex - 1] = path;
    }

    for (auto path : tempPaths) {
        assert(!path.empty());
        FilePath fullPath(rootPath);
        FilePath relativePath(path);
        fullPath.concat(relativePath);
        loader->addNextSample(fullPath);
    }
}

void CompiledInstrument::expandAllKV(SamplerErrorContext& err, SInstrumentPtr inst) {
    assert(!inst->wasExpanded);
    inst->global.compiledValues = SamplerSchema::compile(err, inst->global.values);
    inst->master.compiledValues = SamplerSchema::compile(err, inst->master.values);
    //   inst->control.compiledValues = SamplerSchema::compile(inst->control.values);

    for (auto group : inst->groups) {
        group->compiledValues = SamplerSchema::compile(err, group->values);
        for (auto region : group->regions) {
            region->compiledValues = SamplerSchema::compile(err, region->values);
        }
    }
    inst->wasExpanded = true;
}

#if 0
 void CompiledInstrument::extractDefaultPath(const SInstrumentPtr in) {
    auto value = in->control.compiledValues->get(Opcode::DEFAULT_PATH);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::String);
        this->defaultPath = value->string;
    }
 }
#endif
