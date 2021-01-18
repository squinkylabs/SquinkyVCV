

#include "ACDetector.h"
#include "asserts.h"

void runIt(ACDetector& det, int cyclesQuiet, int cyclesActive) {
}

void testACDetector0() {
    ACDetector ac;
    SqInput in;
    const bool b = ac.step(in, 0);
    assert(!b);
}

void testACDetector() {
    testACDetector0();
}