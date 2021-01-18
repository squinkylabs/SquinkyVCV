

#include "ACDetector.h"
#include "asserts.h"

bool runQuiet(ACDetector& det, int cycles) {
    bool ret = false;
    while (cycles--) {
        ret = det.step(0);
    }
    return ret;
}

bool runSignal(ACDetector& det, int cyclesActive, int period, float levelLo, float levelHi) {
    assert(levelLo <= levelHi);
    int ct = 0;
    bool ret = false;
    while (cyclesActive--) {
        if (++ct > period) {
            ct = 0;
        }
        bool h = ct > period / 2;
        float v = h ? levelHi : levelLo;
        ret = det.step(v);
    }
    return ret;
}

void testACDetector0() {
    ACDetector ac;
    const bool b = ac.step(0);
    assert(!b);
}

void testACDetectorAboveThresh() {
    ACDetector ac;
    const bool b = runSignal(ac, 1000, 100, -5, 5);
    assert(b);
}

void testACDetectorBelowThresh() {
    ACDetector ac;
    const bool b = runSignal(ac, 1000, 100, -ac.threshold / 2, ac.threshold / 2);
    assert(!b);
}

void testACDetectorOnOff() {
    ACDetector ac;
    runSignal(ac, 1000, 100, -5, 5);
    const bool b = runQuiet(ac, ac.thresholdPeriod * 2);
    assert(!b);
}

void testACDetector() {
    testACDetector0();
    testACDetectorAboveThresh();
    testACDetectorBelowThresh();
    testACDetectorOnOff();
}