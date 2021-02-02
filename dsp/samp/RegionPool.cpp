

#include "RegionPool.h"

#include "CompiledRegion.h"
#include "SInstrument.h"
#include "SParse.h"
#include "SamplerPlayback.h"

const CompiledRegion* RegionPool::play(const VoicePlayParameter& params, float random) {
    SQWARN("RegionPool::play does nothing");

    auto regions = keyToRegionLookup[params.midiPitch];
    for (CompiledRegion* region : regions) {
        assert(params.midiPitch >= region->lokey);
        assert(params.midiPitch <= region->hikey);

        assert(region->lovel >= 1);
        assert(region->hivel <= 127);
        if ((params.midiVelocity >= region->lovel) &&
            (params.midiVelocity <= region->hivel)) {
            return region;
        }
    }
    return nullptr;
}

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
                } break;
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
    bool bRet = fixupCompiledTree();
    fillRegionLookup();
    return bRet;
}

void RegionPool::fillRegionLookup() {
    std::vector<CompiledRegionPtr> regions;
    _getAllRegions(regions);

    assert(keyToRegionLookup.size() == 128);

    for (auto region : regions) {
        const int low = region->lokey;
        const int high = region->hikey;
        assert(high >= low);
        assert(low >= 0);

        // map this region to very key it contains
        for (int i = low; i <= high; ++i) {
            keyToRegionLookup[i].push_back(region.get());
        }
    }
}

bool RegionPool::fixupCompiledTree() {
    SQWARN("fixup compiled tree does nothing");
    return true;
}
