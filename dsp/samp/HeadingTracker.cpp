
#include "HeadingTracker.h"

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
            }
            else if (nextHeadingsIndex[regionIndex] < 0) {
                nextHeadingsIndex[regionIndex] = headingIndex;
            }
            else {
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
                assert( nextHeadingsIndex[index] >  curHeadingsIndex[index]);
            }
        }
     }



    /*
    //for (auto heading : hl) {
    for (int headingIndex=0; headingIndex < headings.size(); ++headingIndex) {
        const SHeadingPtr heading = headings[headingIndex];

        const int index = int(heading->type);
        if (curHeadingsIndex[index] < 0) {
            curHeadingsIndex[index] = headingIndex;
        } else if (nextHeadingsIndex[index] < 0) {
            nextHeadingsIndex[index] = headingIndex;
        }
    }
    */
}


void HeadingTracker::nextRegion() {

    // first, move to next region.
    const int regionIndex = int(SHeading::Type::Region);
   // const bool hadNext = nextHeadingsIndex[regionIndex] >= 0;
    const int prevNextRegion = nextHeadingsIndex[regionIndex];

    curHeadingsIndex[regionIndex] = nextHeadingsIndex[regionIndex];
    nextHeadingsIndex[regionIndex] = -1;

    // now, if there was a next before, let's look for another
    if (prevNextRegion >= 0) {
        int i = prevNextRegion + 1;
        for (bool done = false; !done; ++i ) {
            if (i >= headings.size()) {
                done = true;
            }
            else {
                auto type = headings[i]->type;
                if (type == SHeading::Type::Region) {
                    nextHeadingsIndex[regionIndex] = i;
                    done = true;
                }
            }
        }
      
    }

    // now that we have advanced region, let's think about moving up others

}