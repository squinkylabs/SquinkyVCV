
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

void CompiledRegion::findValue(SamplerSchema::DiscreteValue& discreteValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode opcode) {
    assert(inputValues);
    auto value = inputValues->get(opcode);
    if (value) {
        assert(value->type == SamplerSchema::OpcodeType::Discrete);
        discreteValue = value->discrete;
    }
}

using Opcode = SamplerSchema::Opcode;

void CompiledRegion::addRegionInfo(SamplerSchema::KeysAndValuesPtr values) {
    //SQINFO("enter addRegionInfo seqPos=%d  len=", sequencePosition, sequenceLength);
    // SQINFO("accepting values: ");
    //values->_dump();

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
    findValue(amp_veltrack, values, SamplerSchema::Opcode::AMP_VELTRACK);
    findValue(trigger, values, SamplerSchema::Opcode::TRIGGER);

    //----------- sample file
  //  std::string baseFileName;
  //  std::string defaultPathName;
    findValue(baseFileName, values, SamplerSchema::Opcode::SAMPLE);
    findValue(defaultPathName, values, SamplerSchema::Opcode::DEFAULT_PATH);
#if 0
    // let's move this to finalize
    FilePath def(defaultPathName);
    FilePath base(baseFileName);
    def.concat(base);
    if (!def.empty()) {
        this->sampleFile = def.toString();
    }
#endif

    // ---- random and RR -------------------
    findValue(lorand, values, SamplerSchema::Opcode::LO_RAND);
    findValue(hirand, values, SamplerSchema::Opcode::HI_RAND);
    findValue(sequencePosition, values, SamplerSchema::Opcode::SEQ_POSITION);
    findValue(sequenceLength, values, SamplerSchema::Opcode::SEQ_LENGTH);

    // -------------------- key switch variables
    findValue(sw_lolast, values, SamplerSchema::Opcode::SW_LOLAST);
    findValue(sw_hilast, values, SamplerSchema::Opcode::SW_HILAST);

    int sw_last = -1;
    findValue(sw_last, values, SamplerSchema::Opcode::SW_LAST);
    if (sw_last >= 0) {
        if (sw_lolast < 0) {
            sw_lolast = sw_last;
        }
        if (sw_hilast < 0) {
            sw_hilast = sw_last;
        }
    }
    findValue(sw_lokey, values, SamplerSchema::Opcode::SW_LOKEY);
    findValue(sw_hikey, values, SamplerSchema::Opcode::SW_HIKEY);
    findValue(sw_default, values, SamplerSchema::Opcode::SW_DEFAULT);

    keySwitched = (sw_lolast < 0);  // if key switching in effect, default to off
    if (!keySwitched && sw_default >= sw_lolast && sw_default <= sw_hilast) {
        keySwitched = true;
    }

    findValue(sw_label, values, SamplerSchema::Opcode::SW_LABEL);

    // ---------- cc
    findValue(hicc64, values, SamplerSchema::Opcode::HICC64_HACK);
    findValue(locc64, values, SamplerSchema::Opcode::LOCC64_HACK);

    findValue(tune, values, SamplerSchema::Opcode::TUNE);
    findValue(volume, values, SamplerSchema::Opcode::VOLUME);

    //SQINFO("leave addRegionInfo seqPos=%d seqLen=%d samp=%s trigger=%d", sequencePosition, sequenceLength, sampleFile.c_str(), trigger);
}

void CompiledRegion::finalize() {
    //SQINFO("finalize pos = %d len=%d", sequencePosition, sequenceLength);
    if (sequencePosition < 0) {
        sequenceLength = 1;
        sequencePosition = 1;
        // SQINFO("sp < 0, so making def");
    }
    //SQINFO("leave finalize pos = %d len=%d", sequencePosition, sequenceLength);

     // let's move this to finalize
    FilePath def(defaultPathName);
    FilePath base(baseFileName);
    def.concat(base);
    if (!def.empty()) {
        this->sampleFile = def;
    }
}

bool CompiledRegion::shouldIgnore() const {
    // Ignore release triggered samples - we don't implement
    bool dontIgnore = trigger == SamplerSchema::DiscreteValue::NONE || trigger == SamplerSchema::DiscreteValue::ATTACK;
    if (dontIgnore) {
        // Ignore samples that only play with damper pedal.
        dontIgnore = (locc64 == 0);
#if 0
        if (!dontIgnore) {
            SQINFO("discarding region for damper pedal");
        }
#endif
    }
    return !dontIgnore;
}

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

// should only be called when overlapping
static CompiledRegion::OverlapPair overlapRangeIntAmount(int alo, int ahi, int blo, int bhi) {
    assert(overlapRangeInt(alo, ahi, blo, bhi));

    const float totalRange = .5f * (1 + (ahi - alo) + 1 + (bhi - blo));

    const int overlapEnd = std::min(ahi, bhi);
    const int overlapStart = std::max(alo, blo);
    const int overlapAmountInt = 1 + overlapEnd - overlapStart;
    assert(overlapAmountInt >= 1);  // we did assert on overlap, after all

    //  const float overlapAmount = float(overlapEnd - overlapStart);
    const float overlapAmount = float(overlapAmountInt);
    const float overlapFloat = overlapAmount / totalRange;
    assert(overlapFloat >= 0);
    assert(overlapFloat <= 1);
    return std::make_pair(overlapAmountInt, overlapFloat);
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

CompiledRegion::OverlapPair CompiledRegion::overlapVelocityAmount(const CompiledRegion& that) const {
    if (!overlapsVelocity(that)) {
        return std::make_pair(0, 0.f);
    }
    return overlapRangeIntAmount(this->lovel, this->hivel, that.lovel, that.hivel);
}

CompiledRegion::OverlapPair CompiledRegion::overlapPitchAmount(const CompiledRegion& that) const {
    if (!overlapsPitch(that)) {
        return std::make_pair(0, 0.f);
    }
    return overlapRangeIntAmount(this->lokey, this->hikey, that.lokey, that.hikey);
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

int CompiledRegion::velRange() const {
    return 1 + hivel - lovel;
}

int CompiledRegion::pitchRange() const {
    return 1 + hikey - lokey;
}

////////////////////////////////////////////////////////////////////////////////////////////

#if 0   // incoming merge conflict
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

#endif
void CompiledRegion::_dump(int depth) const {
    SQINFO("isKeyswitched=%d, sw_lolast=%d sw_hilast=%d", isKeyswitched(), sw_lolast, sw_hilast);
    SQINFO("seq switched = %d seqCtr = %d, seqLen=%d, seqPos=%d", sequenceSwitched, sequenceCounter, sequenceLength, sequencePosition);
    SQINFO("lorand=%.2f hirand=%.2f\n", lorand, hirand);
    SQINFO("lokey=%d hikey=%d center=%d lovel=%d hivel=%d", lokey, hikey, keycenter, lovel, hivel);
    SQINFO("sample=%s", sampleFile.toString().c_str());
    SQINFO("");
}
