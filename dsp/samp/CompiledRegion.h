#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SamplerSchema.h"
#include "SqLog.h"

class SRegion;
class SGroup;
class CompiledRegion;
class CompiledGroup;
class VoicePlayInfo;
class ISamplerPlayback;
using SRegionPtr = std::shared_ptr<SRegion>;
using SGroupPtr = std::shared_ptr<SGroup>;

using CompiledRegionPtr = std::shared_ptr<CompiledRegion>;
using CompiledGroupPtr = std::shared_ptr<CompiledGroup>;
using CompiledGroupPtrWeak = std::weak_ptr<CompiledGroup>;
using VoicePlayInfoPtr = std::shared_ptr<VoicePlayInfo>;
using ISamplerPlaybackPtr = std::shared_ptr<ISamplerPlayback>;

extern int compileCount;

/**
 * All the data that we care about, pulled out of the SRegion we parsed.
 * These live throughout the duration of a compile, and are the basic structures
 * that drive all the generation of players.
 */
class CompiledRegion {
public:
    CompiledRegion(SRegionPtr, CompiledGroupPtr compiledParent, SGroupPtr parsedParent);
    CompiledRegion() {
        SQINFO("Compiled REgion def ctor %p", this);
        ++compileCount;
    }
    virtual ~CompiledRegion() { compileCount--; }

    enum class Type {
        Base,
        RoundRobin,     // group and regions where we RR between the regions in the one group.
        Random,         // group and regions where we random between the regions in the one group.
        GRandom,        // probability is on each group, so multiple groups involved.
        GRoundRobbin
    };
    virtual Type type() const { return Type::Base; }

    bool overlapsPitch(const CompiledRegion&) const;
    bool overlapsVelocity(const CompiledRegion&) const;
    bool overlapsVelocityButNotEqual(const CompiledRegion&) const;
    bool velocityRangeEqual(const CompiledRegion&) const;
    bool pitchRangeEqual(const CompiledRegion&) const;

    // Keys were defaulting to -1, -1, but for drums with no
    // keys at all they were skipped. Better default is "all keys".
#if 1
    int lokey = 0;
    int hikey = 127;
#else
    int lokey = -1;
    int hikey = -1;
#endif
    // int onlykey = -1;           // can't lokey and hikey represent this just fine?
    int keycenter = -1;
    std::string sampleFile;
    int lovel = 1;
    int hivel = 127;

    // assume no valid random data
    float lorand = -1;
    float hirand = -1;

    int seq_position = -1;
    float amp_veltrack = 100;
    float ampeg_release = .001f;

    CompiledGroupPtrWeak weakParent;
    int lineNumber = -1;
    ;

protected:
    CompiledRegion(CompiledRegionPtr);
    CompiledRegion& operator=(const CompiledRegion&) = default;

private:
    static void findValue(int& returnValue, SamplerSchema::Opcode, const SGroup& parent, const SRegion& region);
    static void findValue(float&, SamplerSchema::Opcode, const SGroup& parent, const SRegion& region);
    static void findValue(std::string&, SamplerSchema::Opcode, const SGroup& parent, const SRegion& region);
};

/**
 * We need multi-regions when we run into a group that defines multiple regions
 * that get picked at runtime.
 */
class CompiledMultiRegion : public CompiledRegion {
public:
    /** This constructor makes a multi-region from a group.
     * copies over all the children so that multi region has the same children as the group.
     */
    CompiledMultiRegion(CompiledGroupPtr parent);
    CompiledMultiRegion() {

    }
    const std::vector<CompiledRegionPtr>& getRegions() { return originalRegions; }

    /** can add more regions this way.
     * Usually used with the default constructor.
     */
    void addChild(CompiledRegionPtr);

protected:
    std::vector<CompiledRegionPtr> originalRegions;
};

using CompiledMultiRegionPtr = std::shared_ptr<CompiledMultiRegion>;

class CompiledRoundRobinRegion : public CompiledMultiRegion {
public:
    CompiledRoundRobinRegion(CompiledGroupPtr parent);
    Type type() const override { return Type::RoundRobin; }
};

class CompiledRandomRegion : public CompiledMultiRegion {
public:
    CompiledRandomRegion(CompiledGroupPtr parent);
    CompiledRandomRegion() = default;
    Type type() const override { return Type::Random; }
};

/**
 * Every Compiled Region had a compiled group as a parent
 */
class CompiledGroup {
public:
    /** This is the normal constructor.
     * create a new compiled group from a parsed group.
     */
    CompiledGroup(SGroupPtr);

    /**
     * This constructor only for makins "synthetic" groups.
     * line number may not be exactly right
     */
    CompiledGroup(int line);
    ~CompiledGroup() { compileCount--; }

    bool shouldIgnore() const;
    void addChild(CompiledRegionPtr child) { regions.push_back(child); }
    std::vector<CompiledRegionPtr> regions;
    CompiledRegion::Type type() const;

    int sequence_length = 0;
    // assume no valid random data
    float lorand = -1;
    float hirand = -1;

    const int lineNumber;

private:
    SamplerSchema::DiscreteValue trigger = SamplerSchema::DiscreteValue::NONE;
};
