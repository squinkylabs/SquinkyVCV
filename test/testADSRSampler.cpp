
#include "ADSRSampler.h"
#include "asserts.h"

static float measureRelease(float r)
{
    const float sampleTime = 1 / 44100.f;

    const float minus85Db = (float) AudioMath::gainFromDb(-85);

    ADSRSampler a;

    a.setASec(.001f);
    a.setDSec(.1f);
    a.setS(1);
    a.setRSec(r);
   
    // prime it once to reset state
    float_4 gatesLow = SimdBlocks::maskFalse();
    float_4 x = a.step(gatesLow, sampleTime);
  

    float_4 gates = SimdBlocks::maskTrue();
    simd_assertMask(gates);

    for (bool done = false; !done; ) {
        float_4 x = a.step(gates, sampleTime);
        if (x[0] > .99f) {
            done = true;
        }
    }

    int rCount = 0;
    gates = float_4(0);
    for (bool done = false; !done; ) {
        ++rCount;
        float_4 x = a.step(gates, sampleTime);
        if (x[0] < minus85Db) {
            done = true;
        }
    }

    float release = rCount * sampleTime;

    a.step(gates, sampleTime);

   // SQINFO("leaving measure(%f) with %f", r, release);
    return release;
}

static void testSub(float time, float ptcTolerance = 5.f) {
    float r = measureRelease(time);
    assertClosePct(r, time, ptcTolerance);
  //  printf("time input = %f, measured = %f\n", time, r);
}

static void debug(float seconds) {
    float r = measureRelease(seconds);
    SQINFO("at %f seconds measure %f", seconds, r);
}

#if 0
static void debug() {
    for (float x = .1f; x < 1; x += .1f) {
        debug(x);
    }
    for (float x = 1; x < 10; x += 1) {
        debug(x);
    }
}
#endif

static void test0() {
  //  SQINFO("----------- test0");

  //  debug();

    testSub(.3f);
    testSub(1.f);
    testSub(2.f);
    testSub(4.f);

    testSub(.5f);
    testSub(.6f);

    testSub(.25f);
    testSub(.1f);
    testSub(.01f);
    testSub(.001f, 20);
    testSub(10);
}



void testADSRSampler()
{
   test0();
}