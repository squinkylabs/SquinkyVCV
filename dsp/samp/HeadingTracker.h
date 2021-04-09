#pragma once

#include <vector>

#include "SParse.h"

class HeadingTracker {
public:
    friend class HeadingTrackerTester;
    HeadingTracker(const SHeadingList&);

    // advance the region field, and updates the others
    void nextRegion();

    SHeadingPtr getCurrent(SHeading::Type);
private:
    const SHeadingList& headings;

    /**
     * the entries are indexs into headings.
     * < 0 means "none"
     */
    std::vector<int> curHeadingsIndex;
    std::vector<int> nextHeadingsIndex;  
};