
#include "ADSRSampler.h"
#include "asserts.h"

static float measureRelease(float r)
{
    float sampleTime = 1 / 44100.f;

    float minus85Db = (float) AudioMath::gainFromDb(-85);

    ADSRSampler a;

  
    a.setASec(.01f);
    a.setDSec(.01f);
    a.setS(1);
    a.setRSec(r);
   

    float_4 gates = (float_4(1) > float_4(0));
    simd_assertMask(gates);

   // float rTime = 0;

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

    SQINFO("leaving measure(%f) with %f", r, release);
    return release;
}

#if 0
static void testADSR_lin()
{
    for (float x = .8f; x > .01f; x *= .5f) {
        float r = measureRelease(x, true);
        printf("r(%f) = %f seconds ratio = %f\n", x, r, r / x);
    }
}



static void testADRS4_1()
{
    // These values are just "known goods" from original ADSR4.
    // But we want to preserve these for existing ADSR4 clients
    float r = measureRelease(.8f, false);
    assertClose(r, 2.2, .1);

    r = measureRelease(.4f, false);
    assertClose(r, .04, .01);

    r = measureRelease(.2f, false);
    assertClose(r, .006, .001);
}
#endif

static void testSub(float time) {
    float r = measureRelease(time);
    assertClosePct(r, time, 5.f);
}

static void test0() {
    testSub(1.f);
    testSub(2.f);
    testSub(4.f);

    testSub(.5f);
    testSub(.25f);
}



void testADSRSampler()
{
   test0();
}