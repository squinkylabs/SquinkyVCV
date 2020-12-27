
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

    std::vector< std::vector<CompiledRegionPtr>> pitchVelList(128);
    assert(pitchVelList.size() == 128);
    for (auto group : in->groups) {
        //
        const bool ignoreGroup = shouldIgnoreGroup(group);

        //printf("comp group with %zd regions. ignore = %d\n", group->regions.size(), ignoreGroup);
        //group->_dump();
       
        if (!ignoreGroup) {
            for (auto region : group->regions) {
              //  printf("in region loop\n");
               // region->_dump();
               // printf("there are %lld values, %lld compiledValue\n", region->values.size(), region->compiledValues->_size() );

                CompiledRegionPtr regBase = std::make_shared<CompiledRegion>(region);
                const bool skipRegion = regBase->lokey < 0 || regBase->hikey < regBase->lokey;
                if (!skipRegion) {
                    const int sampleIndex = addSampleFile(regBase->sampleFile);
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

    // OK, now pitchVelList has all the data we need
    for (int pitch = 1; pitch < 128; ++pitch) {

        std::vector<CompiledRegionPtr>& entriesForPitch = pitchVelList[pitch];
        const int numEntries = int(entriesForPitch.size());

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


ISamplerPlaybackPtr CompiledInstrument::playbackMapVelocities(std::vector<CompiledRegionPtr>& entriesForPitch, int midiPitch) {

    // std::sort (myvector.begin()+4, myvector.end(), myfunction);
    std::sort(entriesForPitch.begin(), entriesForPitch.end(), [](const CompiledRegionPtr a, const CompiledRegionPtr b) -> bool {
        bool less = false;
        if (a->lovel < b->lovel) {
            assert(a->hivel < b->lovel);    // not overlapping
            less = true;
        }
        else {
            assert(a->hivel > b->hivel);
        }
        return less;
    });

#if 0
    printf("------\n");
    for (auto x : entriesForPitch) {
        printf("after sort %d,%d\n", x->lovel, x->hivel);
    }
#endif

    auto vs = std::make_shared<VelSwitch>();
    for (int index = 0; index < entriesForPitch.size(); ++index) {
    // void _addIndex(unsigned int index, unsigned int value);
       // vs->_addIndex(index, entriesForPitch[index]->lovel);
        const int sampleIndex = addSampleFile(entriesForPitch[index]->sampleFile);
        ISamplerPlaybackPtr player = std::make_shared<SimpleVoicePlayer>(entriesForPitch[index], sampleIndex, midiPitch);
        vs->addVelocityRange(entriesForPitch[index]->lovel, player);
    }


    return vs;
}



#if 0
void CompiledInstrument::compile(const SInstrumentPtr in) {
    assert(in->wasExpanded);
    assert(false);      // we need new algo here
    for (auto group : in->groups) {
        //
        const bool ignoreGroup = shouldIgnoreGroup(group);

        printf("comp group with %zd regions. ignore = %d\n", group->regions.size(), ignoreGroup);
        group->_dump();
       
        if (ignoreGroup) {
            return;
        }
        for (auto region : group->regions) {
            // printf("compiling region\n");
            compileSub(region);
        }
    }
}
#endif


#if 1   // need to adapt to new object tree
void CompiledInstrument::compileSub(const SRegionPtr region)
{
    assert(false);
}
#else
void CompiledInstrument::compileSub(const SRegionPtr region)
{
    const SRegion& reg = *region;
         
    int lokey = -1;
    int hikey = -1;
    int onlykey = -1;
    int keycenter = -1;
    std::string sampleFile;
    int lowvel = 0;
    int hivel = 127;

    printf("compile Sub\n");
// this may not scale ;-)
    auto value = reg.compiledValues->get(Opcode::LO_KEY);
    if (value) {
        lokey = value->numericInt;
    }
    value = reg.compiledValues->get(Opcode::HI_KEY);
    if (value) {
        hikey = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::SAMPLE);
    if (value) {
        assert(!value->string.empty());
        sampleFile = value->string;
    }
            
    value = reg.compiledValues->get(Opcode::PITCH_KEYCENTER);
    if (value) {
        keycenter = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::LO_VEL);
    if (value) {
        lowvel = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::HI_VEL);
    if (value) {
        hivel = value->numericInt;
    }

    // until we do vel switching, just grab 64
    if ((lowvel > 64) || (hivel < 64)) {
        // printf("rejecting vel layer %d,%d\n", lowvel, hivel); fflush(stdout);
        return;
    }
     printf("compile Sub 2\n");

    if ((lokey >= 0) && (hikey >= 0) && !sampleFile.empty()) {

         printf("compile Sub 3\n");
        const int sampleIndex = addSampleFile(sampleFile);
        for (int key = lokey; key <= hikey; ++key) {
            VoicePlayInfoPtr info = std::make_shared< VoicePlayInfo>();
            info->valid = true;
            // need to add sample index, transpose amount, etc...
     
            info->sampleIndex = sampleIndex;
            if (key != keycenter && (keycenter != -1)) {
                // we really would like the sample rate info here!

                //  float amount = float(key) / float(keycenter);
                int semiOffset = key - keycenter;
                float pitchMul = float(std::pow(2, semiOffset / 12.0));
                // printf("just added amount = %f key = %d, center = %d\n", pitchMul, key, keycenter);
                info->needsTranspose = true;
                info->transposeAmt = pitchMul;
            } else {
                info->needsTranspose = false;
                info->transposeAmt = 1;
            }
            //   printf("faking sample index 1\n");
            //  printf("adding entry for pitch %d, si=%d\n", key, sampleIndex);

            // it we over-write something bad will happen
            auto temp = pitchMap.find(key);
            assert(temp == pitchMap.end());

            pitchMap[key] = info;
                
        }
    }
    else {
        printf("region defined nothing\n");
    } 
}
#endif

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
#if 1
    pitchMap.play(info, midiPitch, midiVelocity);
#else
     if (testMode) {
         info.sampleIndex = 1;
         info.needsTranspose = false;
         info.transposeAmt = 1;
         info.valid = true;
         return;
     }
     info.valid = false;
     auto entry = pitchMap.find(midiPitch);
     if (entry != pitchMap.end()) {
         //assert(false);
         info = *entry->second;         // this requires copy - we could use smart pointers
         
     }
     else printf("pitch %d not found\n", midiPitch);
#endif
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

#if 0
static std::map<std::string, Opcode> opcodes = {
    {"hivel", Opcode::HI_VEL},
    {"lovel", Opcode::LO_VEL},
    {"hikey", Opcode::HI_KEY},
    {"lokey", Opcode::LO_KEY},
    {"pitch_keycenter", Opcode::PITCH_KEYCENTER},
    {"ampeg_release", Opcode::AMPEG_RELEASE},
    {"loop_mode", Opcode::LOOP_MODE},
    {"loop_continuous", Opcode::LOOP_CONTINUOUS},
    {"loop_start", Opcode::LOOP_START},
    {"loop_end", Opcode::LOOP_END},
    {"sample", Opcode::SAMPLE},
    {"pan", Opcode::PAN},
    {"group", Opcode::GROUP},
    {"trigger", Opcode::TRIGGER},
    {"volume", Opcode::VOLUME},
    {"tune", Opcode::TUNE},
    {"offset", Opcode::OFFSET},
    {"polyphony", Opcode::POLYPHONY},
    {"pitch_keytrack", Opcode::PITCH_KEYTRACK},
    {"amp_veltrack", Opcode::AMP_VELTRACK}
};


static std::set<std::string> unrecognized;
#endif


#if 0
static Opcode translate(const std::string& s) {
    auto entry = opcodes.find(s);
    if (entry == opcodes.end()) {
        auto find2 = unrecognized.find(s);
        if (find2 == unrecognized.end()) {
            unrecognized.insert({ s });
            printf("!! unrecognized opcode %s\n", s.c_str());
        }

        return Opcode::NONE;
    } else {
        return entry->second;
    }
}
#endif

#if 0
// TODO: drive wil map like the others?
static DiscreteValue translated(const std::string& s) {
    if (s == "loop_continuous")
        return DiscreteValue::LOOP_CONTINUOUS;
    if (s == "no_loop")
        return DiscreteValue::NO_LOOP;
    if (s == "attack")
        return DiscreteValue::ATTACK;
    if (s == "release")
        return DiscreteValue::RELEASE;

    
    return DiscreteValue::NONE;
}
#endif

#if 0
// TODO: compare this to the spec
static std::map<Opcode, OpcodeType> keyType = {
    {Opcode::HI_KEY, OpcodeType::Int},
    {Opcode::LO_KEY, OpcodeType::Int},
    {Opcode::HI_VEL, OpcodeType::Int},
    {Opcode::LO_VEL, OpcodeType::Int},
    {Opcode::SAMPLE, OpcodeType::String},
    {Opcode::AMPEG_RELEASE, OpcodeType::Float},
    {Opcode::LOOP_MODE, OpcodeType::Discrete},
    {Opcode::PITCH_KEYCENTER, OpcodeType::Int},
    {Opcode::LOOP_START, OpcodeType::Int},
    {Opcode::LOOP_END, OpcodeType::Int},
    {Opcode::PAN, OpcodeType::Int},
    {Opcode::GROUP, OpcodeType::Int},
    {Opcode::TRIGGER, OpcodeType::Discrete},
    {Opcode::VOLUME, OpcodeType::Float},
    {Opcode::TUNE, OpcodeType::Int},
    {Opcode::OFFSET, OpcodeType::Int},
    {Opcode::POLYPHONY, OpcodeType::Int},
    {Opcode::PITCH_KEYTRACK, OpcodeType::Int},
    {Opcode::AMP_VELTRACK, OpcodeType::Float}
};
#endif

#if 0
static void compile(SamplerSchema::KeysAndValuesPtr results, SKeyValuePairPtr input) {
    Opcode opcode = translate(input->key);
    if (opcode == Opcode::NONE) {
        printf("could not translate opcode %s\n", input->key.c_str());
        return;
    }
    auto typeIter = keyType.find(opcode);
    if (typeIter == keyType.end()) {
        printf("could not find type for %s\n", input->key.c_str());
        assert(false);
        return;
    }

    const OpcodeType type = typeIter->second;
    ValuePtr vp = std::make_shared<Value>();
    vp->type = type;
    bool isValid = true;
    switch (type) {
        case OpcodeType::Int:
            try {
                int x = std::stoi(input->value);
                vp->numericInt = x;
            }
            catch (std::exception&) {
                isValid = false;
                printf("could not convert %s to number. key=%s\n", input->value.c_str(), input->key.c_str());
                return;
            }
            break;
        case OpcodeType::Float:
            try {
                float x = std::stof(input->value);
                vp->numericFloat = x;
            }
            catch (std::exception&) {
                isValid = false;
                printf("could not convert %s to number. key=%s\n", input->value.c_str(), input->key.c_str());
                return;
            }
            break;
        case OpcodeType::String:
            vp->string = input->value;
            break;
        case OpcodeType::Discrete:
        {
            DiscreteValue dv = translated(input->value);
            assert(dv != DiscreteValue::NONE);
            vp->discrete = dv;
        }
            break;
        default:
            assert(false);
    }
    if (isValid) {
        results->add(opcode, vp);
    }

}

SamplerSchema::KeysAndValuesPtr compile(const SKeyValueList& inputs) {

    SamplerSchema::KeysAndValuesPtr results = std::make_shared<SamplerSchema::KeysAndValues>();
    for (auto input : inputs) {
        compile(results, input);
    }
    return results;

}
#endif

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