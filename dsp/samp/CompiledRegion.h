#pragma once

#include "SamplerSchema.h"
#include <memory>
#include <string>

class SRegion;
class SGroup;
class CompiledRegion;
class CompiledGroup;
using SRegionPtr = std::shared_ptr<SRegion>;
using SGroupPtr = std::shared_ptr<SGroup>;

using CompiledRegionPtr = std::shared_ptr<CompiledRegion>;
using CompiledGroupPtr = std::shared_ptr<CompiledGroup>;
using CompiledGroupPtrWeak = std::weak_ptr<CompiledGroup>;

extern int compileCount;

class CompiledRegion
{
public:
    CompiledRegion(SRegionPtr, CompiledGroupPtr parent);
    ~CompiledRegion() { compileCount--; }

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

    CompiledGroupPtrWeak weakParent;
    const int lineNumber;
};

class CompiledGroup {
public:
    CompiledGroup(SGroupPtr);
    ~CompiledGroup()  { compileCount--; }

    bool shouldIgnore() const;
    void addChild(CompiledRegionPtr child) { regions.push_back(child); }

    std::vector<CompiledRegionPtr> regions;
private:
   

    SamplerSchema::DiscreteValue trigger = SamplerSchema::DiscreteValue::NONE;
};


