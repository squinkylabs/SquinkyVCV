
#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SParse.h"
#include "WaveLoader.h"

#include <assert.h>
#include <cmath>
#include <set>
#include <string>

namespace ci
{

static std::map<std::string, Opcode> opcodes = {
    {"hivel", Opcode::HI_VEL},
    {"lokey", Opcode::LO_VEL},
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
    {"volume", Opcode::VOLUME}
};

static std::set<std::string> unrecognized;


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
    {Opcode::VOLUME, OpcodeType::Float}
};

static void compile(KeysAndValuesPtr results, SKeyValuePairPtr input) {
    Opcode opcode = translate(input->key);
    auto typeIter = keyType.find(opcode);
    if (typeIter == keyType.end()) {
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
            vp->nonNUmeric = dv;
        }
            break;
        default:
            assert(false);
    }
    if (isValid) {
        results->add(opcode, vp);
    }

}

#if 0
static void compile(KeysAndValuesPtr results, SKeyValuePairPtr input) {
    Opcode o = translate(input->key);
    DiscreteValue dv = translated(input->value);
    ValuePtr vp = std::make_shared<Value>();
    vp->nonNUmeric = dv;
    bool isValid = true;

    if (o == Opcode::SAMPLE) {
        vp->string = input->value;
    }

    // TODO: exceptions don't work in rack
    else if (dv == DiscreteValue::NONE) {
        try {
            int x = std::stoi(input->value);
            vp->numeric = x;
        }
        catch (std::exception& ) {
            isValid = false;
            printf("could not convert %s to number. key=%s\n", input->value.c_str(), input->key.c_str());
        }
    }

    if (isValid) {
        results->add(o, vp);
    }
 
}
#endif

KeysAndValuesPtr compile(const SKeyValueList& inputs) {

    KeysAndValuesPtr results = std::make_shared<KeysAndValues>();
    for (auto input : inputs) {
        compile(results, input);
    }
    return results;

}

void expandAllKV(SInstrumentPtr inst) {
    assert(!inst->wasExpanded);
    inst->global.compiledValues = compile(inst->global.values);
    for (auto group : inst->groups) {
        group->compiledValues = compile(group->values);
        for (auto region : group->regions) {
            region->compiledValues = compile(region->values);
        }
    }
    inst->wasExpanded = true;
}

void CompiledInstrument::compileSub(const SRegionPtr region)
{
    const SRegion& reg = *region;
         
    int lokey = -1;
    int hikey = -1;
    int onlykey = -1;
    int keycenter = -1;
    std::string sampleFile;

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

    if ((lokey >= 0) && (hikey >= 0) && !sampleFile.empty()) {

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

void CompiledInstrument::compile(const SInstrumentPtr in) {
    assert(in->wasExpanded);
    for (auto group : in->groups) {
        //
        printf("comp group with %zd regions\n", group->regions.size());
        for (auto region : group->regions) {
            printf("compiling region\n");
            compileSub(region);
        }
    }
}

CompiledInstrumentPtr CompiledInstrument::CompiledInstrument::make(SInstrumentPtr inst)
{
    assert(!inst->wasExpanded);
    expandAllKV(inst);
    CompiledInstrumentPtr instOut = std::make_shared< CompiledInstrument>();
    instOut->compile(inst);
    return instOut;
}

void CompiledInstrument::getInfo(VoicePlayInfo& info, int midiPitch, int midiVelocity) {
    
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
 }

void CompiledInstrument::setWaves(WaveLoaderPtr loader, const std::string& rootPath) 
{
    std::vector<std::string> tempPaths;
    assert(!rootPath.empty());

    auto num = relativeFilePaths.size();
    tempPaths.resize(num);
    printf("resized to %zd\n", num);
    // index is 1..
    for (auto pathEntry : relativeFilePaths) {
        std::string path = pathEntry.first;
     
        int waveIndex = pathEntry.second;
        printf("in setWaves, entry has %s index = %d\n", path.c_str(), waveIndex);
       // tempPaths.resize(waveIndex);
        assert(waveIndex > 0);
        assert(!path.empty());
        tempPaths[waveIndex - 1] = path;
     
    }
    printf("after fill, size of temp = %zd, started with %zd\n", tempPaths.size(), relativeFilePaths.size());

    for (auto path : tempPaths) {
        printf("temp: %s\n", path.c_str());
    }


    for (auto path : tempPaths) {
        assert(!path.empty());
        loader->addNextSample(rootPath + path);
    }
}


}