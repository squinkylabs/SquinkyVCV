
#include "CompiledInstrument.h"

#include <assert.h>

#include <algorithm>
#include <cmath>
#include <set>
#include <string>

#include "CompiledRegion.h"
#include "InstrumentInfo.h"
#include "PitchUtils.h"
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
    // SQINFO("dump parse from CompiledInstrument::compile");
    // in->_dump();

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

#if 0
static void dumpRegions(const std::vector<CompiledRegionPtr>& inputRegions) {
    int x = 0;
    for (auto reg : inputRegions) {
        printf("    reg[%d] #%d pitch=%d,%d vel=%d,%d\n", x, reg->lineNumber, reg->lokey, reg->hikey, reg->lovel, reg->hivel);
        ++x;
    }
}
#endif

void CompiledInstrument::_dump(int depth) const {
    regionPool._dump(depth);
}

int CompiledInstrument::addSampleFile(const FilePath& fp) {
    std::string s = fp.toString();
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
    if (!result) {
        SQINFO("unexpected compile error");
        if (!instOut->info) {
            instOut->info = std::make_shared<InstrumentInfo>();
        }
        instOut->info->errorMessage = "unknown compile error";
        instOut->_isInError = true;
    }
    assert(instOut->info);
    return instOut;
}

CompiledInstrumentPtr CompiledInstrument::make(const std::string& parseError) {
    CompiledInstrumentPtr instOut = std::make_shared<CompiledInstrument>();
    instOut->info = std::make_shared<InstrumentInfo>();
    instOut->info->errorMessage = parseError;
    instOut->_isInError = true;
    assert(instOut->info);
    return instOut;
}

float CompiledInstrument::velToGain1(int midiVelocity, float veltrack) {
    const float v = float(midiVelocity);
    // const float t = veltrack;
    //  const float x = (v * t / 100.f) + (100.f - t) * (127.f / 100.f);

    // let's simplify for now
    // scale down to -1..1 (and noone ever uses negative)
    veltrack /= 100.f;

    float x = v;  // veloc 1..127

    // temp = 0...1
    auto temp = float(x) / 127.f;

    // warp it, but still 0..1
    temp *= temp;

    float final = (veltrack * temp) + (1 - veltrack);
    return final;
}

float CompiledInstrument::velToGain(int midiVelocity, float veltrack) {
    return velToGain1(midiVelocity, veltrack);
}

void CompiledInstrument::getGain(VoicePlayInfo& info, int midiVelocity, float regionVeltrack, float regionVolumeDb) {
    float regionGainMult = float(AudioMath::gainFromDb(regionVolumeDb));
    info.gain = velToGain(midiVelocity, regionVeltrack) * regionGainMult;
}


void  CompiledInstrument::getPlayPitchOsc(VoicePlayInfo& info, int midiPitch, int tuneCents, WaveLoader* loader, float sampleRate) {
    //   float transpose = 261.626f * waveInfo->getTotalFrameCount() / sampleRate;
    auto waveInfo = loader->getInfo(info.sampleIndex);
    const float baseFreq = sampleRate /  waveInfo->getTotalFrameCount();

    auto ratio = PitchUtils::semitoneToFreqRatio(float(midiPitch));  
    const float targetFreq = 261.626f * ratio / 32;
    const float transposeMult = (targetFreq / baseFreq);

    float transposeVoltage = PitchUtils::freqRatioToSemitone(transposeMult) / 12.f;
    info.transposeV = transposeVoltage;
}

void CompiledInstrument::getPlayPitchNorm(VoicePlayInfo& info, int midiPitch, int regionKeyCenter, int tuneCents, WaveLoader* loader, float sampleRate) {
    // assert(sampleRate > 100);
    // first base pitch
    const int semiOffset = midiPitch - regionKeyCenter;
    if (semiOffset == 0 && tuneCents == 0) {
        info.needsTranspose = false;
#ifdef _SAMPFM
        info.transposeV = 0;
#else
        info.transposeAmt = 1;
#endif
    } else {
        info.needsTranspose = true;
        // maybe in the future we could do this in the v/8 domain?

        const float tuneSemiOffset = float(semiOffset) + float(tuneCents) / 100;
#ifdef _SAMPFM
        const float offsetCV = tuneSemiOffset / 12.f;
        info.transposeV = offsetCV;
#else
        const float pitchMul = float(std::pow(2, tuneSemiOffset / 12.0));
        info.transposeAmt = pitchMul;
#endif
    }

    if (!loader) {
        return;
    }

    auto waveInfo = loader->getInfo(info.sampleIndex);

    // transpose calculation for normal playback
    // do we need to adapt to changed sample rate?
    unsigned int waveSampleRate = loader->getInfo(info.sampleIndex)->getSampleRate();
    if (!AudioMath::closeTo(sampleRate, waveSampleRate, 1)) {
        info.needsTranspose = true;
#ifdef _SAMPFM
        info.transposeV += PitchUtils::freqRatioToSemitone(float(waveSampleRate) / sampleRate) / 12.f;
#else
        info.transposeAmt *= sampleRate / float(waveSampleRate);
#endif
    }
}

bool CompiledInstrument::play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    assert(sampleRate > 100);
    if (ciTestMode != Tests::None) {
        return playTestMode(info, params, loader, sampleRate);
    }
    info.valid = false;
    float r = rand();

    bool didKS = false;
    const CompiledRegion* region = regionPool.play(params, r, didKS);
    if (region) {
        // SQINFO("playing new region:");
        // region->_dump(1);
        info.sampleIndex = region->sampleIndex;
        info.valid = true;
        info.ampeg_release = region->ampeg_release;
        if (region->loopData.oscillator) {
            getPlayPitchOsc(info, params.midiPitch, region->tune, loader, sampleRate);
        } else {
            getPlayPitchNorm(info, params.midiPitch, region->keycenter, region->tune, loader, sampleRate);
        }
        getGain(info, params.midiVelocity, region->amp_veltrack, region->volume);
        info.loopData = region->loopData;
    }
    return didKS;
}

bool CompiledInstrument::playTestMode(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    float release = 0;
    switch (ciTestMode) {
        case Tests::MiddleC:
            release = .6f;
            break;
        case Tests::MiddleC11:
            release = 1.1f;
            break;
        default:
            assert(false);
    }

    assert(params.midiPitch == 60);  // probably a mistake it a tests isn't using this pitch
    info.sampleIndex = 1;
    info.valid = true;
    info.needsTranspose = false;

    info.ampeg_release = release;
#ifdef _SAMPFM
    info.transposeV = 0;
#else
    info.transposeAmt = 1;
#endif
    return false;  // don't care about KS for tests.
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
        FilePath relativePath(path);
        if (relativePath.isAbsolute()) {
            // SQINFO("found an absolute %s", relativePath.toString().c_str());
            loader->addNextSample(relativePath);
        } else {
            FilePath fullPath(rootPath);

            fullPath.concat(relativePath);
            loader->addNextSample(fullPath);
        }
    }
}

void CompiledInstrument::expandAllKV(SamplerErrorContext& err, SInstrumentPtr inst) {
    assert(!inst->wasExpanded);

    for (auto iter : inst->headings) {
        iter->compiledValues = SamplerSchema::compile(err, iter->values);
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
