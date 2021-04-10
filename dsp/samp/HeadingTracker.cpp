
#include "HeadingTracker.h"

bool HeadingTracker::valid() const {
    const int regionHeadingIndex = curHeadingsIndex[int(SHeading::Type::Region)];
    if (regionHeadingIndex < 0) {
        return true;
    }

    const size_t elements = int(SHeading::Type::NUM_TYPES);
    for (int i = 0; i < elements; ++i) {
        if (i == int(SHeading::Type::Region)) {
            if (curHeadingsIndex[i] != regionHeadingIndex) {
                return false;
            }
        } else {
            if (curHeadingsIndex[i] >= regionHeadingIndex) {
                // parents must come before region
                return false;
            }
        }
    }

    // TODO: next should all be invalied

    return true;
}

SHeadingPtr HeadingTracker::getCurrent(SHeading::Type t) {
    const int typeIndex = int(t);
    const int curIndex = curHeadingsIndex[typeIndex];
    if (curIndex < 0) {
        return nullptr;
    }

    assert(curIndex < headings.size());
    return headings[curIndex];
}

HeadingTracker::HeadingTracker(const SHeadingList& hl) : headings(hl) {
    const size_t elements = int(SHeading::Type::NUM_TYPES);
    curHeadingsIndex.resize(elements, -1);
    nextHeadingsIndex.resize(elements, -1);

    const int regionIndex = int(SHeading::Type::Region);
    // first fill the regions slots, if possible
    for (int headingIndex = 0; headingIndex < headings.size(); ++headingIndex) {
        const SHeadingPtr heading = headings[headingIndex];
        if (heading->type == SHeading::Type::Region) {
            //const int index = int(SHeading::Type::Region);
            if (curHeadingsIndex[regionIndex] < 0) {
                curHeadingsIndex[regionIndex] = headingIndex;
            } else if (nextHeadingsIndex[regionIndex] < 0) {
                nextHeadingsIndex[regionIndex] = headingIndex;
            } else {
                break;
            }
        }
    }

    // new fill the non-region slots
    for (int headingIndex = 0; headingIndex < headings.size(); ++headingIndex) {
        const SHeadingPtr heading = headings[headingIndex];
        if (heading->type != SHeading::Type::Region) {
            const int index = int(heading->type);

            // if we haven't found a heading of this type for our first slot
            if (curHeadingsIndex[index] < 0) {
                // if this index is before our region, and can parent it
                if (headingIndex < curHeadingsIndex[regionIndex]) {
                    curHeadingsIndex[index] = headingIndex;
                }
            } else if ((nextHeadingsIndex[index] < 0) && (headingIndex > curHeadingsIndex[index])) {
                // if we need a next, anything after prev is ok
                nextHeadingsIndex[index] = headingIndex;
                assert(nextHeadingsIndex[index] > curHeadingsIndex[index]);
            }
        }
    }

    assert(valid());
}

void HeadingTracker::nextRegion() {
    // first, move to next region.
    const int regionIndex = int(SHeading::Type::Region);
    // const bool hadNext = nextHeadingsIndex[regionIndex] >= 0;
    const int prevNextRegion = nextHeadingsIndex[regionIndex];

    // move next to cur
    curHeadingsIndex[regionIndex] = nextHeadingsIndex[regionIndex];
    //  nextHeadingsIndex[regionIndex] = -1;


    const int curRegionHeadingIndex = curHeadingsIndex[regionIndex];

    // now that we have advanced region, let's think about moving up others
    const size_t elements = int(SHeading::Type::NUM_TYPES);
    for (int i = 0; i < elements; ++i) {
        // if we are a region parent
        if (i != regionIndex) {
            // and we aren't exhausted yet
            if (curHeadingsIndex[i] >= 0) {
                // Now, see if the NEXT heading is valid. If so that means the current one isn't, and we need to move up
                // and grab the next heading.
                if ((nextHeadingsIndex[i] >= 0) &&  (nextHeadingsIndex[i] < curRegionHeadingIndex)) {
                    curHeadingsIndex[i] = nextHeadingsIndex[i];
                }
            }
        }
    }

    // now let's update all the "next"
    for (int iType = 0; iType < elements; ++iType) {
        // if cur is still active, but next is now out of date
        if (curHeadingsIndex[iType] >= 0 && (nextHeadingsIndex[iType] == curHeadingsIndex[iType])) {
            const SHeading::Type headingType = SHeading::Type(iType);
            int nextIndexCandidate = nextHeadingsIndex[iType] + 1;
            nextHeadingsIndex[iType] = -1;  // clear it out before search
            for (bool done = false; !done; ++nextIndexCandidate) {
                if (nextIndexCandidate >= headings.size()) {
                    done = true;  // we ran out
                } else {
                    auto type = headings[nextIndexCandidate]->type;
                    if (type == headingType) {
                        bool acceptThisOne = false;
                        // ok, we found the next one of our type. Let's see if it's acceptable
                        if (type == SHeading::Type::Region) {
                            done = true;
                            acceptThisOne = true;
                        } else {
                            // we we found on, and it's not too far, we keep it (but we need to stop anyway!
                            if (nextIndexCandidate < curRegionHeadingIndex) {
                                acceptThisOne = true;
                            }
                            done = true;
                        }
                        assert(done);
                        if (acceptThisOne) {
                            nextHeadingsIndex[iType] = nextIndexCandidate;
                            //   if (nextHeadingsIndex[iType] <

                            // nextHeadingsIndex[regionIndex] = i;
                            // done = true;
                        }
                    }
                }
            }
        }
    }

    assert(valid());
}