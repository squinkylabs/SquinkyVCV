

#include "RegionPool.h"

#include "CompiledRegion.h"
#include "SInstrument.h"
#include "SParse.h"
#include "SamplerPlayback.h"

// #define _LOGOV

bool RegionPool::checkPitchAndVel(const VoicePlayParameter& params, const CompiledRegion* region, float random) {
    bool passesCheck = false;
#ifdef _SFZ_RANDOM
    if ((params.midiVelocity >= region->lovel) &&
        (params.midiVelocity <= region->hivel) &&
        (random >= region->lorand) &&
        (random <= region->hirand)) {
        passesCheck = true;
    }
#else
    if ((params.midiVelocity >= region->lovel) &&
        (params.midiVelocity <= region->hivel)) {
        passesCheck = true;
    }
#endif
    return passesCheck;
}

const CompiledRegion* RegionPool::play(const VoicePlayParameter& params, float random) {
    // printf("\n... play(%d)\n", params.midiPitch);
    assert(params.midiPitch >= 0 && params.midiPitch <= 127 && params.midiVelocity > 0 && params.midiVelocity <= 127);

    // First the keyswitch logic from sfizz
    if (!lastKeyswitchLists_[params.midiPitch].empty()) {
        // printf("!lastKeyswitchLists_[%d].empty()\n", params.midiPitch);

        if (currentSwitch_ >= 0 && currentSwitch_ != params.midiPitch) {
            for (auto& region : lastKeyswitchLists_[currentSwitch_]) {
                // SQINFO("setting region->keySwitched = false (turning off regions from the old keyswitch set) r=%p", region);
                region->keySwitched = false;
            }
        }
        currentSwitch_ = params.midiPitch;
        // SQINFO("setting currentSwitch to %d", params.midiPitch);
    }

    for (auto& region : lastKeyswitchLists_[params.midiPitch]) {
        // SQINFO("setting region keyswitched true because in lastKeySwitches");
        region->keySwitched = true;
    }

    // now the region search logic we always had
    CompiledRegion* foundRegion = nullptr;
    const CompiledRegionList& regions = noteActivationLists_[params.midiPitch];
    for (CompiledRegion* region : regions) {
        assert(params.midiPitch >= region->lokey);
        assert(params.midiPitch <= region->hikey);
        assert(region->lovel >= 0);
        assert(region->hivel <= 127);

        // printf("in play loop, looking at region: \n");
        // region->_dump(0);

        bool sequenceMatch = true;
        if (region->sequenceLength > 1) {
            // Sequence activation
            // TODO: do we really need to use sequenceSwitched? might have no use for us
            // WE need to do the calculation for all seq regions, or else they will never come on

            // fprintf(stderr, "region %p", region);
            // fprintf(stderr, "looking for seq ctr=%d len=%d pos=%d\n", region->sequenceCounter, region->sequenceLength, region->sequencePosition);

            sequenceMatch =
                ((region->sequenceCounter++ % region->sequenceLength) == region->sequencePosition - 1);
            // fprintf(stderr, "result: sw=%d ctr=%d\n", sequenceMatch, region->sequenceCounter);
        }

        const bool keyswitched = region->isKeyswitched();
        if (sequenceMatch && !foundRegion && keyswitched && checkPitchAndVel(params, region, random)) {
            foundRegion = region;
        }
    }
    return foundRegion;
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
            for (auto reg : group->regions) {
                auto cReg = std::make_shared<CompiledRegion>(reg, cGroup, group);
                maybeAddToKeyswitchList(cReg);
                if (cReg->sw_default >= 0) {
                    currentSwitch_ = cReg->sw_default;
                }
                regions.push_back(cReg);
            }
        }
    }
    bool bRet = fixupCompiledTree();
    fillRegionLookup();
    return bRet;
}

void RegionPool::maybeAddToKeyswitchList(CompiledRegionPtr region) {
    if (region->sw_lolast >= 0 && region->sw_hilast >= region->sw_lolast) {
        for (int pitch = region->sw_lolast; pitch <= region->sw_hilast; ++pitch) {
            lastKeyswitchLists_[pitch].push_back(region.get());
        }
    }
}

void RegionPool::fillRegionLookup() {
    sortByPitchAndVelocity(regions);
    removeOverlaps();
    assert(noteActivationLists_.size() == 128);

    for (auto region : regions) {
        const int low = region->lokey;
        const int high = region->hikey;
        assert(high >= low);
        assert(low >= 0);

        // map this region to very key it contains
        for (int i = low; i <= high; ++i) {
            noteActivationLists_[i].push_back(region.get());
        }
    }
}

// #define _LOGOV

void RegionPool::removeOverlaps() {
#ifdef _LOGOV
    printf("enter remove overlaps there are %d regions\n",
           (int)regions.size());
    _dump(0);
    printf("\n\n");
#endif
    int removed = 0;
    if (regions.size() < 2) {
        //printf("leaving early, not enough regions\n");
        return;
        //return removed;
    }
    sortByPitchAndVelocity(regions);
    using iterator = std::vector<CompiledRegionPtr>::iterator;
    for (iterator it = regions.begin(); it != regions.end();) {
        iterator itNext = it + 1;
        if (itNext == regions.end()) {
            //return removed;
            //printf("leaving remove at 143 with %d regions", (int)regions.size());
            return;
        }
        CompiledRegionPtr first = *it;
        CompiledRegionPtr second = *itNext;
#ifdef _LOGOV
        printf("overlap comparing line %d with %d\n", first->lineNumber, second->lineNumber);
        printf("  first pitch=%d,%d, vel=%d,%d\n", first->lokey, first->hikey, first->lovel, first->hivel);
        printf("  second pitch=%d,%d, vel=%d,%d\n", second->lokey, second->hikey, second->lovel, second->hivel);

        printf("  first sw_ range=%d, %d. second=%d, %d", first->sw_lolast, first->sw_hilast, second->sw_lolast, second->sw_hilast);
        printf("  overlap pitch = %d, overlap vel = %d\n", first->overlapsPitch(*second), first->overlapsVelocity(*second));
#endif
        if (first->overlapsPitch(*second) &&
            first->overlapsVelocity(*second) &&
            first->overlapsRand(*second) &&
            first->sameSequence(*second)) {
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
#ifdef _LOGOV
    printf("leaving remove overlaps there are %d regions\n", (int)regions.size());
#endif
    return;
}

bool RegionPool::fixupCompiledTree() {
    // TODO: do we need this function any more?
    // SQWARN("fixup compiled tree does nothing");
    return true;
}

void RegionPool::_dump(int depth) const {
    for (int i = 0; i < depth; ++i) {
        printf(" ");
    }
    for (auto region : regions) {
        region->_dump(depth + 4);
    }
}
