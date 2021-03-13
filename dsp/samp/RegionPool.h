#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <array>

class CompiledRegion;
class CompiledGroup;
class SInstrument;
class VoicePlayParameter;

using CompiledRegionPtr = std::shared_ptr<CompiledRegion>;
using CompiledGroupPtr = std::shared_ptr<CompiledGroup>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

class RegionPool {
public:
    /**
     * this is the main "do everything" function
     * that builds up the pool.
     */
    bool buildCompiledTree(const SInstrumentPtr i);

    /** 
     * After the pool is built, this function is called 
     * every time a note needs to be played.
     */
    const CompiledRegion* play(const VoicePlayParameter& params, float random);

    void _dump(int depth) const;
    void _getAllRegions(std::vector<CompiledRegionPtr>&) const;
    static void sortByVelocity(std::vector<CompiledRegionPtr>&);
    static void sortByPitch(std::vector<CompiledRegionPtr>&);
    static void sortByPitchAndVelocity(std::vector<CompiledRegionPtr>&);

    using RegionVisitor = std::function<void(CompiledRegion*)>;

    void visitRegions(RegionVisitor) const;

private:
    std::vector<CompiledRegionPtr> regions;
    bool fixupCompiledTree();
    /**
    * we use raw pointers here.
    * Everything in these lists is kept alive by the object
    * tree from this->groups.
    * noteActivationLists_ is named after the similar variable in sfizz.
    * It tracks for each midi pitch, what regions might play if that key is active.
    */
    using CompiledRegionList = std::vector<CompiledRegion*>;
    std::vector<CompiledRegionList> noteActivationLists_{128};
    std::array<CompiledRegionList, 128> lastKeyswitchLists_;  

    /** current keyswitch value, or -1 if none
     */
    int currentSwitch_ = -1;

    void fillRegionLookup();
    void removeOverlaps();
    void maybeAddToKeyswitchList(CompiledRegionPtr);
 //   static bool checkPitchAndVel(const VoicePlayParameter& params, const CompiledRegion* region, float random);
    static bool shouldRegionPlayNow(const VoicePlayParameter& params, const CompiledRegion* region, float random);

    /**
     * returns true if overlap cannot be corrected.
     * If overlap can be corrected, regions will be tweaked and false will be returned;
     */
    static bool evaluateOverlapsAndAttemptRepair( CompiledRegionPtr firstRegion, CompiledRegionPtr secondRegion);
    //static bool evaluateOverlapsAndAttemptRepairOnce( CompiledRegionPtr firstRegion, CompiledRegionPtr secondRegion);
    static bool attemptOverlapRepairWithVel(CompiledRegionPtr firstRegion, CompiledRegionPtr secondRegion);
    static bool attemptOverlapRepairWithPitch(CompiledRegionPtr firstRegion, CompiledRegionPtr secondRegion);
};