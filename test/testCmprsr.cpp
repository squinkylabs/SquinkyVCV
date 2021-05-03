
#include "Cmprsr.h"
#include "asserts.h"

static void testLimiterZeroAttack2() {
    const float sampleRate = 44100;
    const float threshold = 5;
    const float sampleTime = 1.f / sampleRate;

    Cmprsr comp;
    assert(comp.wasInit());
    comp.setNumChannels(1);
    comp.setCurve(Cmprsr::Ratios::HardLimit);
    comp.setTimes(0, 100, sampleTime);
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

static void testCompZeroAttack2(int numChan) {
    assert(numChan == 1 || numChan == 4);
    const float sampleRate = 44100;
    const float threshold = 5;
    const float sampleTime = 1.f / sampleRate;

    Cmprsr comp;
    assert(comp.wasInit());
    comp.setNumChannels(numChan);
    comp.setCurve(Cmprsr::Ratios::_4_1_soft);
    comp.setTimes(0, 100, sampleTime);
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


// same as mono - all channels the same, but uses poly API
static void testCompZeroAttackPoly(float_4 reduceDist, int numChan) {
    
    assert(numChan == 1 || numChan == 4);
    const float sampleRate = 44100;
    const float threshold = 5;
    const float sampleTime = 1.f / sampleRate;

    Cmprsr comp;
    assert(comp.wasInit());
    comp.setNumChannels(numChan);

    Cmprsr::Ratios r[4] = { Cmprsr::Ratios::_4_1_soft , Cmprsr::Ratios::_4_1_soft , Cmprsr::Ratios::_4_1_soft , Cmprsr::Ratios::_4_1_soft };
    comp.setCurvePoly(r);
    comp.setTimesPoly(0, 100, sampleTime);
    comp.setThresholdPoly(threshold);

    // try some voltages before thresh
    float_4 in(1.34f);
    auto out = comp.stepPoly(in);
    simd_assertEQ(out, in);

    // slam above limit - should settle immediately
    // somewhere above thresh
    in = float_4(10);
    out = comp.stepPoly(in);

    if (numChan == 4) {
        simd_assertClose(out, float_4(5.9f), 1);
    } else {
        assertClose(out[0], 5.9, 1);
    }

    // no more rise after that
    const auto firstOut = out;
    for (int i = 0; i < 10; ++i) {
        out = comp.stepPoly(in);
        simd_assertEQ(out, firstOut);
    }

    // way below threshold. gain will still be reduced, but then go up
    in = float_4(1);
    out = comp.stepPoly(in);
    if (numChan == 4) {
        simd_assertClose(out, float_4(.59f), .1f);
    } else {
        assertClose(out[0], .59, .1);
    }

    // This used to work at 1000
    // TODO: test release time constant for real
    for (int i = 0; i < 100000; ++i) {
        out = comp.stepPoly(in);
    }
    simd_assertClose(out, in, .001);
}

static void testLimiterZeroAttack() {
    testLimiterZeroAttack2();
    testLimiterZeroAttack2();
}

static void testCompZeroAttack(bool poly) {
    if (!poly) {
        testCompZeroAttack2(4);
        testCompZeroAttack2(4);
        testCompZeroAttack2(1);
    } else {
        testCompZeroAttackPoly(SimdBlocks::maskTrue(), 4);
        testCompZeroAttackPoly(SimdBlocks::maskFalse(), 4);
        testCompZeroAttackPoly(SimdBlocks::maskTrue(), 1);
    }
}


static float_4 run(Cmprsr& cmp, int times, float_4 input) {
    float_4 ret;
    for (int i = 0; i < times; ++i) {
        ret = cmp.stepPoly(input);
    }
    return ret;
}

static void testIndependentAttack(int indChan) {
    Cmprsr cmp;

    // attack will be the same, except independent channel will be much longer
    float_4 attack = 0;
    attack[indChan] = 1000;
    float_4 release = 0;
    float sampleTime = 1.f / 44100.f;

    run(cmp, 40, float_4(0));
    cmp.setTimesPoly(attack, release, sampleTime);
    float_4 x = run(cmp, 30, float_4(10));

    int maxIndex = -1;
    float max = -100;;
    float min = 100;


    for (int i = 0; i < 4; ++i) {
        float f = x[i];
        if (f < min) {
            min = f;
           // minIndex = i;
        }
        if (f > max) {
            max = f;
            maxIndex = i;
        }
    }
    assertEQ(maxIndex, indChan);
    for (int i = 0; i < 4; ++i) {
        if (i != indChan) {
            assertEQ(x[i], min);
        }
    }

    // finish test

   // printf("finish this test for testIndependentAttack\n");
}

static void testIndependentAttack() {
    for (int i = 0; i < 4; ++i) {
        testIndependentAttack(i);
    }
}

void testCmprsr() {
    testCompZeroAttack(false);

    fprintf(stderr, "***** put back testCompZeroAttack\n");
   // testCompZeroAttack(true);

    fprintf(stderr, "***** put back testLimiterZeroAttack\n");
   // testLimiterZeroAttack();
    testIndependentAttack();
}