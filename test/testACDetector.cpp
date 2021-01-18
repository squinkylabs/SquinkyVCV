

#include "ACDetector.h"
#include "asserts.h"

bool runQuiet(ACDetector& det, int cycles) {
    bool ret = false;
    while (cycles--) {
        ret |= det.step(0);
    }
    return ret;
}

bool runSignal(ACDetector& det, int cyclesActive, int period) {
    int ct = 0;
    bool ret = false;
    while (cyclesActive--) {
        if (++ct > period) {
            ct = 0;
        }
        bool h = ct > period / 2;
        float v = h ? 5.f : -5.f;
        ret |= det.step(v);
    }
    return ret;
}

void testACDetector0() {
    ACDetector ac;
    const bool b = ac.step(0);
    assert(!b);
}


void testACDetector2() {
    ACDetector ac;

   //const bool b = runIt(ac, in, 0, 1000, 40, 0);
    const bool b = runSignal(ac, 1000, 100);
    assert(b);
}


void testACDetector() {
    testACDetector0();
    testACDetector2();
}