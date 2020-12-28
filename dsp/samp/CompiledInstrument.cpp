
#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "CompiledRegion.h"
#include "SamplerPlayback.h"
#include "SParse.h"
#include "VelSwitch.h"
#include "WaveLoader.h"

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <set>
#include <string>

using Opcode = SamplerSchema::Opcode;
using OpcodeType = SamplerSchema::OpcodeType;
using DiscreteValue = SamplerSchema::DiscreteValue;
using ValuePtr = SamplerSchema::ValuePtr;
using Value = SamplerSchema::Value;

// #define _LOG

/**
 * new compile algorithm
 * 1) filter the regions, and generate a list of compiled regions, each one for a specific pitch
 *  compileRegionList(std::vector<CimpiledREgionPtr>).
 * 
 * std::vector< std::vector<CompiledRegions
 * 
 * x[pitch] is a vector of compiled regions for a pitch
 * 
 */
void CompiledInstrument::compile(const SInstrumentPtr in) {
    assert(in->wasExpanded);

   // std::vector< std::vector<CompiledRegionPtr>> pitchVelList(128);
    PitchVelList  pitchVelList(128);
    assert(pitchVelList.size() == 128);

    for (auto group : in->groups) {
        addGroupToPitchList(pitchVelList, group);
    }

    // OK, now pitchVelList has all the data we need
    for (int pitch = 0; pitch < 128; ++pitch) {

        std::vector<CompiledRegionPtr>& entriesForPitch = pitchVelList[pitch];
        const int numEntries = int(entriesForPitch.size());
#ifdef _LOG
        printf("looping in compile pitches %d, num=%d\n", pitch, numEntries);
#endif

        if (numEntries == 0) {
            // do nothing if play data for this pitch
        } else if (numEntries == 1) {
            const int sampleIndex = addSampleFile(entriesForPitch[0]->sampleFile);
            ISamplerPlaybackPtr playback = std::make_shared<SimpleVoicePlayer>(entriesForPitch[0], sampleIndex, pitch);
            pitchMap.addEntry(pitch, playback);
        }
        else {
            // this is going to sort the entries
            ISamplerPlaybackPtr player = playbackMapVelocities(entriesForPitch, pitch);
            pitchMap.addEntry(pitch, player);
        }
    }
}

void CompiledInstrument::addGroupToPitchList(PitchVelList& pitchVelList, SGroupPtr group) {
  //
    const bool ignoreGroup = shouldIgnoreGroup(group);
  
#ifdef _LOG
    printf("comp group with %zd regions. ignore = %d\n", group->regions.size(), ignoreGroup);
    group->_dump();
#endif
    
    if (!ignoreGroup) {
        CompiledGroupPtr compiledGroup = std::make_shared<CompiledGroup>(group);
        this->groups.push_back(compiledGroup);
        for (auto region : group->regions) {
#ifdef _LOG
            printf("in region loop\n");
            region->_dump();
            printf("there are %lld values, %lld compiledValue\n", region->values.size(), region->compiledValues->_size() );
#endif

            CompiledRegionPtr regBase = std::make_shared<CompiledRegion>(region, compiledGroup);
            const bool skipRegion = regBase->lokey < 0 || regBase->hikey < regBase->lokey;
            if (!skipRegion) {
                // const int sampleIndex = addSampleFile(regBase->sampleFile);
                for (int key = regBase->lokey; key <= regBase->hikey; ++key) {
                    assert(key >= 0 && key <= 127);
                    // printf("in key loop %d\n", key); fflush(stdout);
                    std::vector<CompiledRegionPtr>& vels = pitchVelList[key];
                    
                    vels.push_back(regBase);
                    // printf("vels[%d] has %zd entried\n", key, vels.size());
                }
            }
        }
    }   
}


ISamplerPlaybackPtr CompiledInstrument::playbackMapVelocities(std::vector<CompiledRegionPtr>& entriesForPitch, int midiPitch) {

    std::sort(entriesForPitch.begin(), entriesForPitch.end(), [](const CompiledRegionPtr a, const CompiledRegionPtr b) -> bool {
        bool less = false;
        if (a->lovel < b->lovel) {
            // Let's allow overlap - just sort by lovel
            // assert(a->hivel < b->lovel);    // not overlapping
            less = true;
        }
        else {
            // assert(a->hivel > b->hivel);
        }
        return less;
    });

#ifdef _LOG
    printf("------\n");
    for (auto x : entriesForPitch) {
        printf("after vel sort sort %d,%d\n", x->lovel, x->hivel);
    }
#endif

    auto vs = std::make_shared<VelSwitch>();
    for (int index = 0; index < int(entriesForPitch.size()); ++index) {
    // void _addIndex(unsigned int index, unsigned int value);
       // vs->_addIndex(index, entriesForPitch[index]->lovel);
        const int sampleIndex = addSampleFile(entriesForPitch[index]->sampleFile);
        ISamplerPlaybackPtr player = std::make_shared<SimpleVoicePlayer>(entriesForPitch[index], sampleIndex, midiPitch);
        vs->addVelocityRange(entriesForPitch[index]->lovel, player);
    }


    return vs;
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

bool CompiledInstrument::shouldIgnoreGroup(SGroupPtr group) {
    bool ignore = false;
    auto value = group->compiledValues->get(Opcode::TRIGGER);
    if (value) {
        assert(value->type == OpcodeType::Discrete);
        auto trigger = value->discrete;
        ignore = (trigger != DiscreteValue::ATTACK);
    }
    return ignore;
}

CompiledInstrumentPtr CompiledInstrument::CompiledInstrument::make(SInstrumentPtr inst)
{
    assert(!inst->wasExpanded);
    expandAllKV(inst);
    CompiledInstrumentPtr instOut = std::make_shared< CompiledInstrument>();
    instOut->compile(inst);
    return instOut;
}

void CompiledInstrument::play(VoicePlayInfo& info, int midiPitch, int midiVelocity) {
    pitchMap.play(info, midiPitch, midiVelocity);
}

void CompiledInstrument::setWaves(WaveLoaderPtr loader, const std::string& rootPath) 
{
    std::vector<std::string> tempPaths;
    assert(!rootPath.empty());

    auto num = relativeFilePaths.size();
    tempPaths.resize(num);
    // printf("resized to %zd\n", num);
    // index is 1..
    for (auto pathEntry : relativeFilePaths) {
        std::string path = pathEntry.first;
     
        int waveIndex = pathEntry.second;
        //printf("in setWaves, entry has %s index = %d\n", path.c_str(), waveIndex);
       // tempPaths.resize(waveIndex);
        assert(waveIndex > 0);
        assert(!path.empty());
        tempPaths[waveIndex - 1] = path;
     
    }
#if 0
    printf("after fill, size of temp = %zd, started with %zd\n", tempPaths.size(), relativeFilePaths.size());

    for (auto path : tempPaths) {
        printf("temp: %s\n", path.c_str());
    }
#endif

    for (auto path : tempPaths) {
        assert(!path.empty());
        loader->addNextSample(rootPath + path);
    }
}

void CompiledInstrument::expandAllKV(SInstrumentPtr inst) {
    assert(!inst->wasExpanded);
    inst->global.compiledValues = SamplerSchema::compile(inst->global.values);
    for (auto group : inst->groups) {
        group->compiledValues = SamplerSchema::compile(group->values);
        for (auto region : group->regions) {
            region->compiledValues = SamplerSchema::compile(region->values);
        }
    }
    inst->wasExpanded = true;
}
