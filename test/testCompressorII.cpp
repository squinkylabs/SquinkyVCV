
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
    assertEQ(bool(std::round(comp.params[Comp2::STEREO_PARAM].value)), false);
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

    auto holder = comp.getParamHolder();
    assertEQ(comp.params[Comp2::THRESHOLD_PARAM].value, 10.f);
}


static void testUnLinked()
{
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

static void testLinked()
{
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

void testCompressorII( ) {
    testMB_1();
    testUnLinked();
    testLinked();
   
}