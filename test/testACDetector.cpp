

#include "ACDetector.h"
#include "asserts.h"

bool runIt(ACDetector& det, SqInput& in, int cyclesQuiet, int cyclesActive, int periodActive, int cyclesQuiet2) {
    bool ret = false;
    while (cyclesQuiet--) {
        in.setVoltage(0, 0);
        det.step(in, 1);
    }
    int ct = 0;
    while (cyclesActive--) {
        if (++ct > periodActive) {
            ct = 0;
        }
        bool h = ct > periodActive / 2;
        in.setVoltage(h ? 5.f : -5.f, 0);
        ret = det.step(in, 1);
    }
    while (cyclesQuiet2--) {
        in.setVoltage(0, 0);
        ret = det.step(in, 1);
    }
    return ret;
}

void testACDetector0() {
    ACDetector ac;
    SqInput in;
    const bool b = ac.step(in, 1);
    assert(!b);
}

void testACDetector1() {
    ACDetector ac;
    SqInput in;
    in.setChannels(0);
    assert(!in.isConnected());
    const bool b = runIt(ac, in, 0, 1000, 40, 0);
    assert(!b); 
}


void testACDetector2() {
    ACDetector ac;
    SqInput in;
    in.channels = 1;            // pretend that it's hooked up
    in.setChannels(1);
    assert(in.isConnected());
    const bool b = runIt(ac, in, 0, 1000, 40, 0);
    assert(b);
}


void testACDetector() {
    testACDetector0();
    testACDetector1();
    testACDetector2();
}