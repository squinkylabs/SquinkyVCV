#pragma once

#include <vector>

#include "SParse.h"

class HeadingTracker {
public:
    friend class HeadingTrackerTester;
    HeadingTracker(const SHeadingList&);

private:
    std::vector<const SHeading*> curHeadings;
    std::vector<const SHeading*> nextHeadings;
};