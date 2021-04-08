
#include "HeadingTracker.h"

HeadingTracker::HeadingTracker(const SHeadingList& hl) {
    const size_t elements = int(SHeading::Type::NUM_TYPES);
    curHeadings.resize(elements, nullptr);
    nextHeadings.resize(elements, nullptr);

    for (auto heading : hl) {
        const int index = int(heading->type);
        if (!curHeadings[index]) {
            curHeadings[index] = heading.get();
        } else if (!nextHeadings[index]) {
            nextHeadings[index] = heading.get();
        }
    }
}