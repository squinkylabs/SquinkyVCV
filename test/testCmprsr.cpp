
#include "asserts.h"
#include "Cmprsr.h"



static void testLimiterZeroAttack(bool reduceDist) {
    const float sampleRate = 44100;
    const float threshold = 5;
    const float sampleTime = 1.f / sampleRate;

    Cmprsr comp;
    assert(comp.wasInit());
    comp.setNumChannels(1);
    comp.setCurve(Cmprsr::Ratios::HardLimit);
    comp.setTimes(0, 100, sampleTime, reduceDist);
    comp.setThreshold(threshold);

    // below thresh, no reduction
    float_4 in(1.34f);
    auto out = comp.step(in);
    simd_assertEQ(out, in);

    // slam above limit - should limit immediately
    // by setting gain to one half
    in = float_4(10);
    out = comp.step(in);
    simd_assertEQ(out, float_4(threshold));

    // way below threshold. gain will still be reduced, but then go up
    // but at first is still one half
    in = float_4(1);
    out = comp.step(in);
    simd_assertClose(out, float_4(.5), .001f);

    // This used to work at 1000
    // TODO: test release time constant for real
    for (int i = 0; i < 100000; ++i) {
        out = comp.step(in);
    }
    simd_assertClose(out, in, .001);
}

static void testCompZeroAttack(bool reduceDist, int numChan) {
    assert(numChan == 1 || numChan == 4);
    const float sampleRate = 44100;
    const float threshold = 5;
    const float sampleTime = 1.f / sampleRate;

    Cmprsr comp;
    assert(comp.wasInit());
    comp.setNumChannels(numChan);
    comp.setCurve(Cmprsr::Ratios::_4_1_soft);
    comp.setTimes(0, 100, sampleTime, reduceDist);
    comp.setThreshold(threshold);

    // try some voltages before thresh
    float_4 in(1.34f);
    auto out = comp.step(in);
    simd_assertEQ(out, in);

    // slam above limit - should settle immediately
    // somewhere above thresh
    in = float_4(10);
    out = comp.step(in);

    if (numChan == 4) {
        simd_assertClose(out, float_4(5.9f), 1);
    } else {
        assertClose(out[0], 5.9, 1);
    }

    // no more rise after that
    const auto firstOut = out;
    for (int i = 0; i < 10; ++i) {
        out = comp.step(in);
        simd_assertEQ(out, firstOut);
    }

    // way below threshold. gain will still be reduced, but then go up
    in = float_4(1);
    out = comp.step(in);
    if (numChan == 4) {
        simd_assertClose(out, float_4(.59f), .1f);
    } else {
        assertClose(out[0], .59, .1);
    }

    // This used to work at 1000
    // TODO: test release time constant for real
    for (int i = 0; i < 100000; ++i) {
        out = comp.step(in);
    }
    simd_assertClose(out, in, .001);
}

static void testLimiterZeroAttack() {
    testLimiterZeroAttack(false);
    testLimiterZeroAttack(true);
}

static void testCompZeroAttack() {
    testCompZeroAttack(true, 4);
    testCompZeroAttack(false, 4);
    testCompZeroAttack(true, 1);
}


void testCmprsr()
{
    testCompZeroAttack();
    testLimiterZeroAttack();
}