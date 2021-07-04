
#include "SqLog.h"

#include <assert.h>
#include <cinttypes>
#include <cmath>
#include <limits>
#include <vector>

/**
 * Algorithm
 *  add next entry to next row.
 *  while (!good) try other possibilities for next row
 *  if can't find any, backtrack.
 * q: how do we know what we tried back there?
 */

#if 0
class GcTracker {
public:
    GcTracker() = default;
    void make();
private:
    void makeNext();
    uint16_t table[0x10000];
    int curIndex = 0;
};

void GcTracker::make() {
    for (bool done = false; !done; ) {
        if (curIndex >= std::numeric_limits<uint16_t>::max()) {
            done = true;
        }
        else {
            makeNext();
        }
    }
}
void GcTracker::makeNext() {
    if (curIndex == 0) {
        table[curIndex++] = 0;
        return;
    }

    assert(false);  // what now?

}
#endif


class GcGenerator {
public:
    using NumType = uint16_t;       // let's limit ourselves to 16 bit
    GcGenerator(int nBits);
 //   void make();
     void dump();
private:
    const int numBits;
    int numEntries;
    NumType currentBit = 0;         // which bit we are trying in current state
    void makeNextGuess();


   
    std::vector<NumType> state;
    std::vector<NumType> data;      // the generated dat
};

inline GcGenerator::GcGenerator(int nBits) : numBits(nBits) {
    SQINFO("----- generating %d bits -------");
    assert(numBits <= 16);
    assert((numBits & 1) == 0);
    state.push_back(0);         // initial guess
    numEntries = int(std::round(std::pow<int>(2, numBits)));
    SQINFO(" numEntreis = %d", numEntries);

    makeNextGuess();
    data = state;
    assert(state.size() == numEntries);
    assert(data.size() == numEntries);
}

inline void GcGenerator::makeNextGuess() {
    SQINFO("makeNext Guess, state=%d cb=%d", state.size(), currentBit);
    if (state.size() == numEntries) {
        SQINFO("all done");
        return;
    }
    if (currentBit < numBits-1) {
        currentBit++;
        makeNextGuess();
    } else {
        state.back() = currentBit;
        state.push_back(0);
        currentBit = 0;
        makeNextGuess();
    }
}

inline void GcGenerator::dump() {
    for (int i=0; i< numEntries; ++i) {
        SQINFO("%x", data[i]);
    }
}

#if 0
void GcGenerator::make() {
    makeNextGuess();
}
#endif


void gc_fill() {
    GcGenerator gc(4);
    gc.dump();

}


void do_gc() {
    gc_fill();
    assert(false);
 
}