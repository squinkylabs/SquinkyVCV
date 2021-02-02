#pragma once

#include <vector>
#include <memory>

class CompiledRegion;
class CompiledGroup;
class SInstrument;

using CompiledRegionPtr = std::shared_ptr<CompiledRegion>;
using CompiledGroupPtr = std::shared_ptr<CompiledGroup>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

class RegionPool {
public:
    void _getAllRegions(std::vector<CompiledRegionPtr>&) const;
    const std::vector<CompiledGroupPtr>& _groups() const;
    static void sortByVelocity(std::vector<CompiledRegionPtr>&);
    static void sortByPitch(std::vector<CompiledRegionPtr>&);
    static void sortByPitchAndVelocity(std::vector<CompiledRegionPtr>&);

    bool buildCompiledTree(const SInstrumentPtr i);
    
private:
     std::vector<CompiledGroupPtr> groups;
     bool fixupCompiledTree();

};