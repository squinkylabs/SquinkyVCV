#pragma once

#include <memory>
#include <string>
#include <vector>

#include "FilePath.h"
#include "SamplerSchema.h"
#include "SqLog.h"

class CompiledRegion;
class VoicePlayInfo;
class ISamplerPlayback;

using CompiledRegionPtr = std::shared_ptr<CompiledRegion>;

//using CompiledGroupPtr = std::shared_ptr<CompiledGroup>;
//using CompiledGroupPtrWeak = std::weak_ptr<CompiledGroup>;

using VoicePlayInfoPtr = std::shared_ptr<VoicePlayInfo>;
using ISamplerPlaybackPtr = std::shared_ptr<ISamplerPlayback>;

extern int compileCount;

#define _SFZ_RANDOM

/**
 * All the data that we care about, pulled out of the SRegion we parsed.
 * These live throughout the duration of a compile, and are the basic structures
 * that drive all the generation of players.
 */
class CompiledRegion {
public:
    void addRegionInfo(SamplerSchema::KeysAndValuesPtr);

    // line numbers here are one based, but coming in are zero
    CompiledRegion(int ln) : lineNumber(ln + 1) {
        // SQINFO("Compiled REgion def ctor %p", this);
        ++compileCount;
    }
    virtual ~CompiledRegion() { compileCount--; }
    void _dump(int depth) const;

    int velRange() const;
    int pitchRange() const;

    bool overlapsPitch(const CompiledRegion&) const;
    bool overlapsVelocity(const CompiledRegion&) const;
    bool overlapsVelocityButNotEqual(const CompiledRegion&) const;
    bool velocityRangeEqual(const CompiledRegion&) const;
    bool pitchRangeEqual(const CompiledRegion&) const;
    bool overlapsRand(const CompiledRegion&) const;
    bool sameSequence(const CompiledRegion&) const;

    /** 
     * Find out by how much two regsions overlap.
     * result = <intergerAmount:floatRatio>
     * 
     * float ratio 1.0 means complete overlap
     * float ratio  0 means no overlap at all
    */
    using OverlapPair = std::pair<int, float>;

    OverlapPair overlapVelocityAmount(const CompiledRegion&) const;
    OverlapPair overlapPitchAmount(const CompiledRegion&) const;

    int lokey = 0;
    int hikey = 127;

    int keycenter = 60;

    int lovel = 1;
    int hivel = 127;

    // assume no valid random data
    float lorand = 0;
    float hirand = 1;

    float amp_veltrack = 100;
    float ampeg_release = .03f;  // correct default, not .001

    int lineNumber = -1;  // one based

    /** valid sample index starts at 1
     */
    int sampleIndex = 0;

    FilePath sampleFile;
    std::string baseFileName;
    std::string defaultPathName;

    /**
     * Member variable to control round robin selection
     * of regions. Variable are named after the corresponding variables
     * from sfizz. . Def to true, set to false for sequence groups.
     */
    bool sequenceSwitched = true;
    int sequenceCounter = 0;  //: int region member, init to zero.
    int sequenceLength = 1;   // uint8_t init to 1, set  from sfz data
    int sequencePosition = -1;

    /**
     * for key switching
     */
    bool keySwitched = true;  // by default, normal regions are on
                              //  int sw_last = -1;         // the pitch that turns on this region
    int sw_lolast = -1;       // the range of pitches that turn this region on
    int sw_hilast = -1;

    int sw_lokey = -1;
    int sw_hikey = -1;    // the range of pitches that are key-switches, not notes
    int sw_default = -1;  // the keyswitch region to start with
    std::string sw_label;

    // cc stuff
    int hicc64 = 127;
    int locc64 = 0;

    float volume = 0;  // volume change in db
    int tune = 0;      // tuning offset in cents

    /**
     * LoopData is broken out just to give a little structure.
     * Note that not all the entires are strictly about looping, but 
     * they are all about playing ranges of samples
     */
    class LoopData {
    public:
        bool operator==(const LoopData&) const;
        unsigned int offset = 0;
        unsigned int end = 0;
        unsigned int loop_start = 0;
        unsigned int loop_end = 0;
        SamplerSchema::DiscreteValue loop_mode = SamplerSchema::DiscreteValue::NO_LOOP;
        bool oscillator = false;
    };

    LoopData loopData;

    SamplerSchema::DiscreteValue trigger = SamplerSchema::DiscreteValue::NONE;

    bool isKeyswitched() const {
        return keySwitched;
    }

    /**
     * Returns true if the settings in the region lead us to determine it
     * does not, or should not, make sound.
     */
    bool shouldIgnore() const;

    /** 
     * should be called after everyone has called 
     * addRegionInfo
     */
    void finalize();

protected:
    CompiledRegion(CompiledRegionPtr);
    CompiledRegion& operator=(const CompiledRegion&) = default;

private:
    static void findValue(float& returnValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode);
    static void findValue(int& returnValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode);
    static void findValue(bool& returnValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode);
    static void findValue(unsigned int& returnValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode);

    static void findValue(std::string& returnValue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode);
    static void findValue(SamplerSchema::DiscreteValue& returnVAlue, SamplerSchema::KeysAndValuesPtr inputValues, SamplerSchema::Opcode);
};
