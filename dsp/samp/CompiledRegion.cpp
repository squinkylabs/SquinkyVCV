
#include "CompiledRegion.h"

#include <functional>
#include <set>

#include "CompiledInstrument.h"
#include "FilePath.h"
#include "SParse.h"
#include "SqLog.h"
#include "SamplerSchema.h"

int compileCount = 0;

void CompiledRegion::findValue(int& intValue, SamplerSchema::Opcode opcode, const SGroup& parent, const SRegion& reg) {
    assert(&parent);
    auto value = reg.compiledValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Int);
        intValue = value->numericInt;
        return;
    }
    value = parent.compiledValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Int);
        intValue = value->numericInt;
        return;
    }
    return;
}

void CompiledRegion::findValue(float& floatValue, SamplerSchema::Opcode opcode, const SGroup& parent, const SRegion& reg) {
    auto value = reg.compiledValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Float);
        floatValue = value->numericFloat;
        return;
    }
    value = parent.compiledValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Float);
        floatValue = value->numericFloat;
        return;
    }
    return;
}

void CompiledRegion::findValue(std::string& strValue, SamplerSchema::Opcode opcode, const SGroup& parent, const SRegion& reg) {
    auto value = reg.compiledValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::String);
        strValue = value->string;
        return;
    }
    value = parent.compiledValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::String);
        strValue = value->string;
        return;
    }
    return;
}

void findValue(float&, SamplerSchema::Opcode) {}
void findValue(std::string&, SamplerSchema::Opcode) {}

using Opcode = SamplerSchema::Opcode;
CompiledRegion::CompiledRegion(SRegionPtr region, CompiledGroupPtr compiledParent, SGroupPtr parsedParent) : weakParent(compiledParent), lineNumber(region->lineNumber) {
    assert(parsedParent);
    compileCount++;
    const SRegion& reg = *region;
    assert(reg.compiledValues);

    // TODO: better API for getting values
    findValue(lokey, SamplerSchema::Opcode::LO_KEY, *parsedParent, reg);
    findValue(hikey, SamplerSchema::Opcode::HI_KEY, *parsedParent, reg);

    int key = -1;
    findValue(key, SamplerSchema::Opcode::KEY, *parsedParent, reg);
    if (key >= 0) {
        lokey = hikey = keycenter = key;
    }
#if 0
    {
        SQINFO("in cr::cr of region %p) here is parent:", this);
        parsedParent->_dump();
        SQINFO(" and here is reg (still in ctor)");
        region->_dump();

    }
#endif
    findValue(seq_position, SamplerSchema::Opcode::SEQ_POSITION, *parsedParent, reg);
    findValue(keycenter, SamplerSchema::Opcode::PITCH_KEYCENTER, *parsedParent, reg);
    findValue(lovel, SamplerSchema::Opcode::LO_VEL, *parsedParent, reg);
    findValue(hivel, SamplerSchema::Opcode::HI_VEL, *parsedParent, reg);

    findValue(lorand, SamplerSchema::Opcode::LO_RAND, *parsedParent, reg);
    findValue(hirand, SamplerSchema::Opcode::HI_RAND, *parsedParent, reg);

    findValue(amp_veltrack, SamplerSchema::Opcode::AMP_VELTRACK, *parsedParent, reg);
    findValue(ampeg_release, SamplerSchema::Opcode::AMPEG_RELEASE, *parsedParent, reg);

    std::string baseFileName;
    std::string defaultPathName;
    findValue(baseFileName, SamplerSchema::Opcode::SAMPLE, *parsedParent, reg);
    findValue(defaultPathName, SamplerSchema::Opcode::DEFAULT_PATH, *parsedParent, reg);

    FilePath def(defaultPathName);
    FilePath base(baseFileName);
    def.concat(base);
    this->sampleFile = def.toString();

   // findValue(sampleFile, SamplerSchema::Opcode::SAMPLE, *parsedParent, reg);
}

static bool overlapRange(int alo, int ahi, int blo, int bhi) {
    assert(alo <= ahi);
    assert(blo <= bhi);
#if 0  // original version
    return (blo >= alo && blo <= ahi) ||   // blo is in A
        (bhi >= alo && bhi <= ahi);         // or bhi is in A
#else
    return (blo <= ahi && bhi >= alo) ||
           (alo <= bhi && ahi >= blo);
#endif
}

bool CompiledRegion::overlapsPitch(const CompiledRegion& that) const {
    return overlapRange(this->lokey, this->hikey, that.lokey, that.hikey);
}

bool CompiledRegion::overlapsVelocity(const CompiledRegion& that) const {
    return overlapRange(this->lovel, this->hivel, that.lovel, that.hivel);
}

bool CompiledRegion::overlapsVelocityButNotEqual(const CompiledRegion& that) const {
    return overlapsVelocity(that) && !velocityRangeEqual(that);
}

bool CompiledRegion::velocityRangeEqual(const CompiledRegion& that) const {
    return (this->lovel == that.lovel) && (this->hivel == that.hivel);
}

bool CompiledRegion::pitchRangeEqual(const CompiledRegion& that) const {
    return (this->lokey == that.lokey) && (this->hikey == that.hikey);
}

CompiledRegion::CompiledRegion(CompiledRegionPtr prototype) {
    compileCount++;
    *this = *prototype;
}

///////////////////////////////////////////////////////////////////////////////////////

CompiledMultiRegion::CompiledMultiRegion(CompiledGroupPtr parent) : CompiledRegion(parent->regions[0]) {
    for (auto region : parent->regions) {
        originalRegions.push_back(region);
    }
}

void CompiledMultiRegion::addChild(CompiledRegionPtr child)
{
    originalRegions.push_back(child);
}

CompiledRoundRobinRegion::CompiledRoundRobinRegion(CompiledGroupPtr parent) : CompiledMultiRegion(parent){};

CompiledRandomRegion::CompiledRandomRegion(CompiledGroupPtr parent) : CompiledMultiRegion(parent) {
}

////////////////////////////////////////////////////////////////////////////////////////////

CompiledGroup::CompiledGroup(SGroupPtr group) : lineNumber(group->lineNumber) {
    compileCount++;

    auto value = group->compiledValues->get(Opcode::TRIGGER);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Discrete);
        //ignore = (trigger != DiscreteValue::ATTACK);
        trigger = value->discrete;
    }

    value = group->compiledValues->get(Opcode::SEQ_LENGTH);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Int);
        sequence_length = value->numericInt;
    }
    value = group->compiledValues->get(Opcode::LO_RAND);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Float);
        lorand = value->numericFloat;
    }
    value = group->compiledValues->get(Opcode::HI_RAND);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Float);
        hirand = value->numericFloat;
    }
}

CompiledGroup::CompiledGroup(int line) : lineNumber(line) {
    compileCount++;
}

bool CompiledGroup::shouldIgnore() const {
    bool dontIgnore = trigger == SamplerSchema::DiscreteValue::NONE || trigger == SamplerSchema::DiscreteValue::ATTACK;
    return !dontIgnore;
}

CompiledRegion::Type CompiledGroup::type() const {
    CompiledRegion::Type theType = CompiledRegion::Type::Base;
    if (this->sequence_length > 0) {
        theType = CompiledRegion::Type::RoundRobin;
    } else if (this->lorand >= 0) {
       // the group has prob on it.
        theType = CompiledRegion::Type::GRandom;
    } else {
        bool isProbabilty = !regions.empty();  // assume if any regions we are a probability group
        for (auto child : regions) {
            // lorand=0 hirand=0.3
            if (child->lorand < 0) {
                isProbabilty = false;
            }
        }
        if (isProbabilty) {
            theType = CompiledRegion::Type::Random;
            if (regions.size() < 2) SQWARN("rand region no options");
            //assert(regions.size() > 1);
        }
    }
    return theType;
}