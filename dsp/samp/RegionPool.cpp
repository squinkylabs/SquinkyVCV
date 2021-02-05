

#include "RegionPool.h"

#include "CompiledRegion.h"
#include "SInstrument.h"
#include "SParse.h"
#include "SamplerPlayback.h"

const CompiledRegion* RegionPool::play(const VoicePlayParameter& params, float random) {
    SQWARN("RegionPool::play does nothing");

    const CompiledRegionList& regions = keyToRegionLookup[params.midiPitch];
    for (CompiledRegion* region : regions) {
        assert(params.midiPitch >= region->lokey);
        assert(params.midiPitch <= region->hikey);

        assert(region->lovel >= 0);
        assert(region->hivel <= 127);

#ifdef _SFZ_RANDOM
        if ((params.midiVelocity >= region->lovel) &&
            (params.midiVelocity <= region->hivel) &&
            (random >= region->lorand) &&
            (random <= region->hirand)) {
                  return region;
            }
#else
        if ((params.midiVelocity >= region->lovel) &&
            (params.midiVelocity <= region->hivel)) {
            return region;
        }
#endif
    }
    return nullptr;
}

// TODO: reduce code with the visitor
void RegionPool::_getAllRegions(std::vector<CompiledRegionPtr>& array) const {
#if 0
    assert(array.empty());
    for (auto group : groups) {
        for (auto region : group->regions) {
            array.push_back(region);
        }
    }
#endif
    array = regions;
}

void RegionPool::visitRegions(RegionVisitor visitor) const {
    for (auto region : regions) {
        visitor(region.get());
    }
}

void RegionPool::sortByVelocity(std::vector<CompiledRegionPtr>& array) {
    std::sort(array.begin(), array.end(), [](const CompiledRegionPtr a, const CompiledRegionPtr b) -> bool {
        bool less = false;
        if (a->lovel < b->lovel) {
            less = true;
        }
        return less;
    });
}

void RegionPool::sortByPitch(std::vector<CompiledRegionPtr>&) {
    assert(false);
}

void RegionPool::sortByPitchAndVelocity(std::vector<CompiledRegionPtr>& array) {
    std::sort(array.begin(), array.end(), [](const CompiledRegionPtr a, const CompiledRegionPtr b) -> bool {
        bool less = false;
        if (a->lokey < b->lokey) {
            less = true;
        } else if (a->lokey == b->lokey) {
            less = (a->lovel < b->lovel);
        }
        return less;
    });
}

bool RegionPool::buildCompiledTree(const SInstrumentPtr in) {
    for (auto group : in->groups) {
        auto cGroup = std::make_shared<CompiledGroup>(group);
        if (!cGroup->shouldIgnore()) {
            //this->groups.push_back(cGroup);
            for (auto reg : group->regions) {
                auto cReg = std::make_shared<CompiledRegion>(reg, cGroup, group);
                // cGroup->addChild(cReg);
                regions.push_back(cReg);
            }
        }
    }
    bool bRet = fixupCompiledTree();
    fillRegionLookup();
    return bRet;
}

void RegionPool::fillRegionLookup() {
    sortByPitchAndVelocity(regions);
    removeOverlaps();

    // TODO: remove overlap

    // TODO: what about bad groups?

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

void RegionPool::removeOverlaps() {
#ifdef _LOGOV
    printf("enter remove overlaps\n");
#endif
    int removed = 0;
    if (regions.size() < 2) {
        return;
        //return removed;
    }
    sortByPitchAndVelocity(regions);
    using iterator = std::vector<CompiledRegionPtr>::iterator;
    for (iterator it = regions.begin(); it != regions.end();) {
        iterator itNext = it + 1;
        if (itNext == regions.end()) {
            //return removed;
            return;
        }
        CompiledRegionPtr first = *it;
        CompiledRegionPtr second = *itNext;
#ifdef _LOGOV
        printf("overlap comparing line %d with %d\n", first->lineNumber, second->lineNumber);
        printf("  first pitch=%d,%d, vel=%d,%d\n", first->lokey, first->hikey, first->lovel, first->hivel);
        printf("  second pitch=%d,%d, vel=%d,%d\n", second->lokey, second->hikey, second->lovel, second->hivel);
        printf("  overlap pitch = %d, overlap vel = %d\n", first->overlapsPitch(*second), first->overlapsVelocity(*second));
#endif
        if (first->overlapsPitch(*second) && first->overlapsVelocity(*second)) {
            // keep the region with the smallest pitch range
            const int firstPitchRange = first->hikey - first->lokey;
            const int secondPitchRange = second->hikey - second->lokey;
            if (firstPitchRange <= secondPitchRange) {
#ifdef _LOGOV
                printf("about to erase region from %d based on conflict from %d\n", second->lineNumber, first->lineNumber);
#endif
                // if we want to erase the second one, do that.
                // it still points at first, but next iteration there will be a different next;
                regions.erase(itNext);
                ++removed;
            } else {
#ifdef _LOGOV
                printf("about to(b) erase region from %d\n", first->lineNumber);
#endif
                // we erase the first one, leaving
                // it pointing at next.
                // so we are set up to continue loop fine
                it = regions.erase(it);
            }
        } else {
            ++it;
#ifdef _LOGOV
            printf("not removing\n");
#endif
        }
    }
    //return removed;
    return;
}

bool RegionPool::fixupCompiledTree() {
    SQWARN("fixup compiled tree does nothing");
    return true;
}
