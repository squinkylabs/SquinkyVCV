
#include <assert.h>
#include <cinttypes>
#include <limits>

/**
 * Algorithm
 *  add next entry to next row.
 *  while (!good) try other possibilities for next row
 *  if can't find any, backtrack.
 * q: how do we know what we tried back there?
 */


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





void gc_fill() {
    GcTracker gc;
    gc.make();

}


void do_gc() {
    gc_fill();
 
}