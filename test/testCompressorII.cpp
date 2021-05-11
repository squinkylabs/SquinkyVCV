
#include "Compressor2.h"
#include "SqLog.h"
#include "asserts.h"
#include "tutil.h"

using Comp2 = Compressor2<TestComposite>;
static void run(Comp2& comp, int times = 40) {
    TestComposite::ProcessArgs args;
    for (int i = 0; i < times; ++i) {
        comp.process(args);
    }
}

static void init(Comp2& comp) {
    comp.init();
    initComposite(comp);
}

static void testMB_1() {
    Comp2 comp;
    init(comp);
    // run normal
    run(comp);

    // default first channel on
    assertEQ(int(std::round(comp.params[Comp2::CHANNEL_PARAM].value)), 1);
    assertEQ(int(std::round(comp.params[Comp2::STEREO_PARAM].value)), 2);

    // now set to mono, the old default.
    comp.params[Comp2::STEREO_PARAM].value = 0;

    // assert thresh 1 is max (numbering doesn't start at 0, but 1)
    assertEQ(comp.params[Comp2::THRESHOLD_PARAM].value, 10.f);
    // assert threash 2 is max
    comp.params[Comp2::CHANNEL_PARAM].value = 2.f;
    run(comp);
    assertEQ(comp.params[Comp2::THRESHOLD_PARAM].value, 10.f);

    // move back to channel 1, and set to stereo, and take threshold to zero
    comp.params[Comp2::CHANNEL_PARAM].value = 1.f;
    run(comp);
    comp.params[Comp2::STEREO_PARAM].value = 1.f;
    run(comp);
    comp.params[Comp2::THRESHOLD_PARAM].value = 0.f;
    run(comp);
    assertEQ(comp.params[Comp2::THRESHOLD_PARAM].value, 0.f);

    // now go to channel 2. thr should still be high
    comp.params[Comp2::CHANNEL_PARAM].value = 2.f;
    run(comp);

    const CompressorParmHolder& holder = comp.getParamHolder();
    assertEQ(comp.params[Comp2::THRESHOLD_PARAM].value, 10.f);
}

static void testUnLinked() {
    Cmprsr cmp;
    cmp.setIsPolyCV(false);

    cmp.setCurve(Cmprsr::Ratios::_8_1_hard);
    cmp.setTimes(0, 100, 1.f / 44100.f);
    cmp.setThreshold(.1f);

    cmp.setIsPolyCV(true);
    float_4 input(1, 2, 3, 4);
    float_4 x;
    for (int i = 0; i < 10; ++i) {
        x = cmp.stepPoly(input);
    }
    assertClosePct(x[0], x[1], 20);
}

static void testLinked() {
    Cmprsr cmp;
    cmp.setIsPolyCV(false);

    cmp.setCurve(Cmprsr::Ratios::_8_1_hard);
    cmp.setTimes(0, 100, 1.f / 44100.f);
    cmp.setThreshold(.1f);
    cmp.setLinked(true);

    cmp.setIsPolyCV(true);
    float_4 input(1, 2, 3, 4);
    float_4 x;
    for (int i = 0; i < 10; ++i) {
        x = cmp.stepPoly(input);
    }
    x = cmp.stepPoly(input);
    // assertClosePct(x[0], x[1], 20);
    const float in1_ratio = input[1] / input[0];
    const float in2_ratio = input[3] / input[2];
    const float rx1 = x[1] / x[0];
    assertClosePct(x[1] / x[0], in1_ratio, 10);
    assertClosePct(x[3] / x[2], in2_ratio, 10);
}

// test that after we change to stereo that
// all the channels are refreshed, not just the current one

static void testMB_2() {
    Comp2 comp;
    init(comp);

    // force mono, run
    comp.params[Comp2::STEREO_PARAM].value = 0;
    run(comp);

    // verify initial th very high
    for (int i = 0; i < 4; ++i) {
        auto x = comp._getComp(0)._getTh();
        simd_assertEQ(x, float_4(10));
    }

    // set thresh really low on channel 1 and 16
    comp.params[Comp2::CHANNEL_PARAM].value = 1;
    run(comp);
    comp.params[Comp2::THRESHOLD_PARAM].value = 0;
    run(comp);
    comp.params[Comp2::CHANNEL_PARAM].value = 16;
    run(comp);
    comp.params[Comp2::THRESHOLD_PARAM].value = 0;
    run(comp);
    // verify that the theshold is correct
    // the TH we assert against are just "known goods".
    // But the do look reasonable. The important thing
    // is that stereo pairs are the same.
    for (int i = 0; i < 4; ++i) {
        auto x = comp._getComp(i)._getTh();
        switch (i) {
            case 0: {
                float_4 expect(.1f, 10, 10, 10);
                simd_assertEQ(x, expect);
            } break;
            case 3: {
                float_4 expect(10, 10, 10, .1f);
                simd_assertEQ(x, expect);
            } break;

            case 1:
            case 2: {
                float_4 expect(10);
                simd_assertEQ(x, expect);
            } break;
            default:
                assert(false);
        }
    }

    // now set to stereo
    comp.params[Comp2::STEREO_PARAM].value = 1;
    run(comp);

    // verify all th are normalized
    // verify that the theshold is correct
    for (int i = 0; i < 4; ++i) {
        auto x = comp._getComp(i)._getTh();
        switch (i) {
            case 0: {
                float_4 expect(1, 1, 10, 10);
                simd_assertEQ(x, expect);
            } break;
            case 3: {
                float_4 expect(10, 10, 1, 1);
                simd_assertEQ(x, expect);
            } break;
            case 1:
            case 2: {
                float_4 expect(10);
                simd_assertEQ(x, expect);
            } break;
            default:
                assert(false);
        }
    }
}

void testCompressorII() {
    testMB_1();
    testUnLinked();
    testLinked();
    testMB_2();
}