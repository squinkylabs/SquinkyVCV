
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
     if (s == "lokey") 
        return Opcode::LO_KEY;

    return Opcode::NONE;

}

static DiscreteValue translated(const std::string& s) {
    return DiscreteValue::NONE;
}

static void compile(KeysAndValuesPtr results, SKeyValuePairPtr input) {
    Opcode o = translate(input->key);
    DiscreteValue dv = translated(input->value);
    ValuePtr vp = std::make_shared<Value>();
    vp->nonNUmeric = dv;
    if (dv == DiscreteValue::NONE) {
        int x = std::stoi(input->value);
        vp->numeric = x;
    }

    results->add(o, vp);
 
}

KeysAndValuesPtr compile(const SKeyValueList& inputs) {

    KeysAndValuesPtr results = std::make_shared<KeysAndValues>();
    for (auto input : inputs) {
        compile(results, input);
    }
    return results;

}

void expandAllKV(SInstrumentPtr inst) {
    inst->global.compiledValues = compile(inst->global.values);
    for (auto group : inst->groups) {
        group->compiledValues = compile(group->values);
        for (auto region : group->regions) {
            region->compiledValues = compile(region->values);
        }
    }
}

CompiledInstrumentPtr compile(const SInstrumentPtr in) {
    CompiledInstrumentPtr instOut = std::make_shared< CompiledInstrument>();
    return instOut;
}


 void CompiledInstrument::getInfo(VoicePlayInfo&, int midiPitch, int midiVelocity) {
     
 }


}