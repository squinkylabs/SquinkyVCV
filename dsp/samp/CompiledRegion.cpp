
#include "CompiledRegion.h"

#include <functional>
#include <set>

#include "CompiledInstrument.h"
#include "FilePath.h"
#include "SParse.h"
#include "SamplerSchema.h"
#include "SqLog.h"

int compileCount = 0;

void CompiledRegion::findValue(float& floatValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode opcode) {
    assert(inputValues);
    auto value = inputValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Float);
        floatValue = value->numericFloat;
    }
}

void CompiledRegion::findValue(int& intValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode opcode) {
    assert(inputValues);
    auto value = inputValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Int);
        intValue = value->numericInt;
    }
}

void CompiledRegion::findValue(std::string& stringValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode opcode) {
    assert(inputValues);
    auto value = inputValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::String);
        stringValue = value->string;
    }
}

using Opcode = SamplerSchema::Opcode;

void CompiledRegion::addRegionInfo(SamplerSchema::KeysAndValuesPtr values) {
    // TODO: what did old findValue to that we don't?
    // TODO: why did old version need so many args?
    // TODO: do we need weakParent? get rid of it?
    // TODO: line numbers

    //---------------- key related values
    findValue(lokey, values, SamplerSchema::Opcode::LO_KEY);
    findValue(hikey, values, SamplerSchema::Opcode::HI_KEY);

    int key = -1;
    findValue(key, values, SamplerSchema::Opcode::KEY);
    if (key >= 0) {
        lokey = hikey = keycenter = key;
    }
    // TODO: only do this if key not set?
    findValue(keycenter, values, SamplerSchema::Opcode::PITCH_KEYCENTER);

    //---------------------------------------------velocity
    findValue(lovel, values, SamplerSchema::Opcode::LO_VEL);
    findValue(hivel, values, SamplerSchema::Opcode::HI_VEL);
    assert(lovel >= 0);  // some idiot instruments use zero, even though it is not legal
    assert(hivel >= lovel);
    assert(hivel <= 127);

    //------------- misc
    findValue(ampeg_release, values, SamplerSchema::Opcode::AMPEG_RELEASE);

    //----------- sample file
    std::string baseFileName;
    std::string defaultPathName;
    findValue(baseFileName, values, SamplerSchema::Opcode::SAMPLE);
    findValue(defaultPathName, values, SamplerSchema::Opcode::DEFAULT_PATH);
    FilePath def(defaultPathName);
    FilePath base(baseFileName);
    def.concat(base);
    this->sampleFile = def.toString();

    //  static void findValue(float& returnValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode);
}

#if 0
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

    // key switch trigger variables
  
    findValue(sw_lolast, SamplerSchema::Opcode::SW_LOLAST, *parsedParent, reg);
    findValue(sw_hilast, SamplerSchema::Opcode::SW_HILAST, *parsedParent, reg);

    int sw_last = -1;
    findValue(sw_last, SamplerSchema::Opcode::SW_LAST, *parsedParent, reg);
    if (sw_last >= 0) {
        if (sw_lolast < 0) {
            sw_lolast = sw_last;
        }
         if (sw_hilast < 0) {
            sw_hilast = sw_last;
        }
    }


    // key switch range variables
    findValue(sw_lokey, SamplerSchema::Opcode::SW_LOKEY, *parsedParent, reg);
    findValue(sw_hikey, SamplerSchema::Opcode::SW_HIKEY, *parsedParent, reg);
    findValue(sw_default, SamplerSchema::Opcode::SW_DEFAULT, *parsedParent, reg);

    keySwitched = (sw_lolast < 0);            // if key switching in effect, default to off
    if (!keySwitched && sw_default >= sw_lolast && sw_default <= sw_hilast) {
        keySwitched = true;
    }

    findValue(sw_label, SamplerSchema::Opcode::SW_LABEL, *parsedParent, reg);

    findValue(sequencePosition, SamplerSchema::Opcode::SEQ_POSITION, *parsedParent, reg);
    findValue(sequenceLength, SamplerSchema::Opcode::SEQ_LENGTH, *parsedParent, reg);

    findValue(keycenter, SamplerSchema::Opcode::PITCH_KEYCENTER, *parsedParent, reg);
    findValue(lovel, SamplerSchema::Opcode::LO_VEL, *parsedParent, reg);
    findValue(hivel, SamplerSchema::Opcode::HI_VEL, *parsedParent, reg);
    assert(lovel >= 0);  // some idiot instruments use zero, even though it is not logal
    assert(hivel >= lovel);
    assert(hivel <= 127);

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

    // if not position set in region, fix it up so it will
    // not be part of sequence logic.
    if (sequencePosition < 0) {
        // assert(sequenceLength == 1);
        sequenceLength = 1;
        sequencePosition = 1;
    }

    // findValue(sampleFile, SamplerSchema::Opcode::SAMPLE, *parsedParent, reg);
}
#endif

#if 0
static bool overlapRange(int alo, int ahi, int blo, int bhi) {
    assert(alo <= ahi);
    assert(blo <= bhi);
    return (blo <= ahi && bhi >= alo) ||
           (alo <= bhi && ahi >= blo);

}
#endif

// Int version: if ranges have a value in common, they overlap
static bool overlapRangeInt(int alo, int ahi, int blo, int bhi) {
    assert(alo <= ahi);
    assert(blo <= bhi);
    return (blo <= ahi && bhi >= alo) ||
           (alo <= bhi && ahi >= blo);
}

// float version: if ranges have a value in common, they don't necessarily overlap
static bool overlapRangeFloat(float alo, float ahi, float blo, float bhi) {
    assert(alo <= ahi);
    assert(blo <= bhi);
    return (blo < ahi && bhi > alo) ||
           (alo < bhi && ahi > blo);
}

bool CompiledRegion::overlapsPitch(const CompiledRegion& that) const {
    // of both regions have valid sw_last info
    if (sw_lolast >= 0 && sw_hilast >= 0 && that.sw_lolast >= 0 && that.sw_hilast >= 0) {
        // and the ranges don't overlap
        bool switchesOverlap = overlapRangeInt(sw_lolast, sw_hilast, that.sw_lolast, that.sw_hilast);
        if (!switchesOverlap) {
            // ... then there can't be a pitch conflict
            return false;
        }
    }

    return overlapRangeInt(this->lokey, this->hikey, that.lokey, that.hikey);
}

bool CompiledRegion::overlapsVelocity(const CompiledRegion& that) const {
    return overlapRangeInt(this->lovel, this->hivel, that.lovel, that.hivel);
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

bool CompiledRegion::overlapsRand(const CompiledRegion& that) const {
    return overlapRangeFloat(this->lorand, this->hirand, that.lorand, that.hirand);
}
bool CompiledRegion::sameSequence(const CompiledRegion& that) const {
    return this->sequencePosition == that.sequencePosition;
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

void CompiledMultiRegion::addChild(CompiledRegionPtr child) {
    originalRegions.push_back(child);
}

CompiledRoundRobinRegion::CompiledRoundRobinRegion(CompiledGroupPtr parent) : CompiledMultiRegion(parent){};

CompiledRandomRegion::CompiledRandomRegion(CompiledGroupPtr parent) : CompiledMultiRegion(parent) {
}

////////////////////////////////////////////////////////////////////////////////////////////

CompiledGroup::CompiledGroup(SGroupPtr group) : lineNumber(group->lineNumber) {
    compileCount++;

    // do we still need all these members in groups?
    // do they really do something anymore?
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

void CompiledRegion::_dump(int depth) const {
    // for (int i=0; i<depth; ++i) {
    //    printf(" ");
    //}
    printf("isKeyswitched=%d, sw_lolast=%d sw_hilast=%d\n", isKeyswitched(), sw_lolast, sw_hilast);
    printf("seq switched = %d seqCtr = %d, seqLen=%d, seqPos=%d\n", sequenceSwitched, sequenceCounter, sequenceLength, sequencePosition);
    printf("lorand=%.2f hirand=%.2f\n", lorand, hirand);
    printf("lokey=%d hikey=%d center=%d lovel=%d hivel=%d\n", lokey, hikey, keycenter, lovel, hivel);
    printf("\n");
}
