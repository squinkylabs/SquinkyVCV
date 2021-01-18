

#include "ACDetector.h"
#include "asserts.h"

static bool runQuiet(ACDetector& det, int cycles) {
    bool ret = false;
    while (cycles--) {
        ret = det.step(0);
    }
    return ret;
}

static bool runSignal(ACDetector& det, int cyclesActive, int period, float levelLo, float levelHi) {
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

static void testACDetector0() {
    ACDetector ac;
    const bool b = ac.step(0);
    assert(!b);
}

static void testACDetectorAboveThresh() {
    ACDetector ac;
    const bool b = runSignal(ac, 1000, 100, -5, 5);
    assert(b);
}

static void testACDetectorBelowThresh() {
    ACDetector ac;
    const bool b = runSignal(ac, 1000, 100, -ac.threshold / 2, ac.threshold / 2);
    assert(!b);
}

static void testACDetectorOnOff() {
    ACDetector ac;
    runSignal(ac, 1000, 100, -5, 5);
    const bool b = runQuiet(ac, ac.thresholdPeriod * 2);
    assert(!b);
}

class ta_vco {
public:
    void setPitch(float pitch) {
        //  delta = std::clamp(pitch, 0.f, .5f);
        delta = pitch;
    }
    float process() {
        acc += delta;
        if (acc > 1) {
            acc -= 1;
        }
        return (float)std::sin(acc * AudioMath::Pi * 2);
    }
    float _getFrequency() const { return delta; }

private:
    float acc = 0;
    float delta = 0;
};

static bool runSin(ACDetector& ac, float pitch, int period) {
    ta_vco sin;
    sin.setPitch(pitch);
    bool ret = false;
    while (period--) {
        float sample = sin.process();
        ret = ac.step(sample);
    }
    return ret;
}

static void testACDetector_hfsin() {
    ACDetector ac;
    const bool b = runSin(ac, .1f, 50);
    assert(b);
}

static void testACDetector_lfsin() {
    ACDetector ac;
    float lf = .1f / ac.thresholdPeriod;
    const bool b = runSin(ac, lf, 44000);
    assert(!b);
}

void testACDetector() {
    testACDetector0();
    testACDetectorAboveThresh();
    testACDetectorBelowThresh();
    testACDetectorOnOff();
    testACDetector_hfsin();
    testACDetector_lfsin();
}