
#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SParse.h"

#include <assert.h>
#include <string>

namespace ci
{

static Opcode translate(const std::string& s) {
    if (s == "hikey")
        return Opcode::HI_KEY;
    else if (s == "lokey")
        return Opcode::LO_KEY;
    else if (s == "pitch_keycenter")
        return Opcode::PITCH_KEYCENTER;
    else if (s == "ampeg_release")
        return Opcode::AMPEG_RELEASE;
    else if (s == "loop_mode")
        return Opcode::LOOP_MODE;
    else if (s == "loop_continuous")
        return Opcode::LOOP_CONTINUOUS;
    else if (s == "loop_start")
        return Opcode::LOOP_START; 
    else if (s == "loop_end")
        return Opcode::LOOP_END;
    else if (s == "sample")
        return Opcode::SAMPLE;

    else printf("!! unrecognized optode %s\n", s.c_str());

    return Opcode::NONE;

}

static DiscreteValue translated(const std::string& s) {
    if (s == "loop_continuous") 
        return DiscreteValue::LOOP_CONTINUOUS;
    if (s == "no_loop")
         return DiscreteValue::NO_LOOP;
    
    return DiscreteValue::NONE;
}

static void compile(KeysAndValuesPtr results, SKeyValuePairPtr input) {
    Opcode o = translate(input->key);
    DiscreteValue dv = translated(input->value);
    ValuePtr vp = std::make_shared<Value>();
    vp->nonNUmeric = dv;
    bool isValid = true;

    if (o == Opcode::SAMPLE) {
        vp->string = input->value;
    }

    // TODO: excpetions don't work in rack
    else if (dv == DiscreteValue::NONE) {
        try {
            int x = std::stoi(input->value);
            vp->numeric = x;
        }
        catch (std::exception& ) {
            isValid = false;
            printf("could not convert %s to number\n", input->value.c_str());
        }
    }

    if (isValid) {
        results->add(o, vp);
    }
 
}

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

void CompiledInstrument::compile(const SInstrumentPtr in) {
    assert(in->wasExpanded);
    for (auto group : in->groups) {
        //
        printf("comp group with %zd regions\n", group->regions.size());
        for (auto region : group->regions) {
            printf("compiling region\n");
            const SRegion& reg = *region;
            VoicePlayInfoPtr info = std::make_shared< VoicePlayInfo>();
            int lokey = -1;
            int hikey = -1;
            int onlykey = -1;
#if 0
            for (auto value : *(reg.compiledValues)) {
                switch (value->key) {
                    case Opcode::LO_KEY:
                        lowkey = value->value;
                        break:
                    default:
                        assert(false);
                }
               
            }
#endif
            auto value = reg.compiledValues->get(Opcode::LO_KEY);
            if (value) {
                lokey = value->numeric;
            }
            value = reg.compiledValues->get(Opcode::HI_KEY);
            if (value) {
                hikey = value->numeric;
            }
#if 0
            value = reg.compiledValues->get(Opcode::KEY);
            if (value) {
                hikey = value->numeric;
            }
#endif
            


            if ((lokey >= 0) && (hikey >= 0)) {
                for (int key = lokey; key <= hikey; ++key) {
                    info->valid = true;
                    // need to add sample index, transpose amount, etc...
                    info->sampleIndex = 1;
                    printf("faking sample index 1\n");
                    printf("adding entry for pitch %d\n", key);
                    pitchMap[key] = info;
                }
            }
            else {
                printf("region defined nothing\n");
            }
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


}