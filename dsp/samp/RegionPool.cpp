

//#include "Compile"
#include "CompiledRegion.h"
#include "RegionPool.h"
#include "SInstrument.h"
#include "SParse.h"

void RegionPool::_getAllRegions(std::vector<CompiledRegionPtr>& array) const {
    assert(array.empty());
    for (auto group : groups) {
        for (auto region : group->regions) {
            array.push_back(region);
        }
    }
}

const std::vector<CompiledGroupPtr>& RegionPool::_groups() const {
    return groups;
}

void RegionPool::sortByVelocity(std::vector<CompiledRegionPtr>&) {

}

void RegionPool::sortByPitch(std::vector<CompiledRegionPtr>&) {

}

void RegionPool::sortByPitchAndVelocity(std::vector<CompiledRegionPtr>&) {

}



bool RegionPool::buildCompiledTree(const SInstrumentPtr in) {
    for (auto group : in->groups) {
        auto cGroup = std::make_shared<CompiledGroup>(group);
        if (!cGroup->shouldIgnore()) {
            this->groups.push_back(cGroup);
            for (auto reg : group->regions) {
                auto cReg = std::make_shared<CompiledRegion>(reg, cGroup, group);
                cGroup->addChild(cReg);
            }
            // we that the tree is build, ask the group it it's special

            const CompiledRegion::Type type = cGroup->type();
            switch (type) {
                case CompiledRegion::Type::Base:
                    // nothing to do - tree is good
                    break;
                case CompiledRegion::Type::Random:
                case CompiledRegion::Type::RoundRobin: {
                    //  cGroup->regions.clear();
                    //  CompiledRegionPtr newRegion = std::make_shared < CompiledRoundRobbinRegion>();
                    //  cGroup->regions.push_back(newRegion);
                  //  bool b = remakeTreeForMultiRegion(type, cGroup);
                 //   if (!b) {
                //        return false;
                    }
                 break;
                case CompiledRegion::Type::GRandom: {
                    // do nothing, we will fixup in a second pass.
                    //bool b = remakeTreeForGMultiRegion();
                } break;
                default:
                    assert(false);
                    return false;
            }
        }
    }
    return fixupCompiledTree();
}

bool RegionPool::fixupCompiledTree() {
    SQWARN("fixup compiled tree does nothing");
    return true;
}
