#pragma once

#include <functional>
#include <memory>
#include <vector>

class CompiledRegion;
class CompiledGroup;
class SInstrument;
class VoicePlayParameter;

using CompiledRegionPtr = std::shared_ptr<CompiledRegion>;
using CompiledGroupPtr = std::shared_ptr<CompiledGroup>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

class RegionPool {
public:
    void _dump(int depth) const;
    const CompiledRegion* play(const VoicePlayParameter& params, float random);

    void _getAllRegions(std::vector<CompiledRegionPtr>&) const;
    //  const std::vector<CompiledGroupPtr>& _groups() const;
    static void sortByVelocity(std::vector<CompiledRegionPtr>&);
    static void sortByPitch(std::vector<CompiledRegionPtr>&);
    static void sortByPitchAndVelocity(std::vector<CompiledRegionPtr>&);

    bool buildCompiledTree(const SInstrumentPtr i);

    using RegionVisitor = std::function<void(CompiledRegion*)>;

    void visitRegions(RegionVisitor) const;

private:
    //  std::vector<CompiledGroupPtr> groups;
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

    void fillRegionLookup();
    void removeOverlaps();
};