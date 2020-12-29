
#include "CompiledInstrument.h"
#include "CompiledRegion.h"

#include "SParse.h"

#include <set>
#include <functional>

int compileCount = 0;

using Opcode = SamplerSchema::Opcode;
CompiledRegion::CompiledRegion(SRegionPtr region, CompiledGroupPtr parent) : weakParent(parent) 
{
    compileCount++;
    const SRegion& reg = *region;
    assert(reg.compiledValues);

    // TODO: better API for getting values
    auto value = reg.compiledValues->get(SamplerSchema::Opcode::LO_KEY);
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
        lovel = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::HI_VEL);
    if (value) {
        hivel = value->numericInt;
    }
}


CompiledGroup::CompiledGroup(SGroupPtr group)
{
    compileCount++;

    auto value = group->compiledValues->get(Opcode::TRIGGER);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Discrete);
        //ignore = (trigger != DiscreteValue::ATTACK);
        trigger =  value->discrete;
    }
}

bool CompiledGroup::shouldIgnore() const
{
    bool dontIgnore = trigger == SamplerSchema::DiscreteValue::NONE || trigger == SamplerSchema::DiscreteValue::ATTACK;
    return !dontIgnore;
}
