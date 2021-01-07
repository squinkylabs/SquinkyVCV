
#include "ADSR4.h"
#include "asserts.h"

static float measureRelease(float R, bool lin)
{
    float sampleTime = 1 / 44100.f;

    float minus80Db = (float) AudioMath::gainFromDb(-80);

    ADSR4 a;

  
    a.setA(0, 1);
    a.setD(0, 1);
    a.setS(1);
    if (lin) {
        a.setR_L(R);
    }
    else {
       
        a.setR(R, 1);
    }

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
        if (x[0] < minus80Db) {
            done = true;
        }
    }

    float release = rCount * sampleTime;

    a.step(gates, sampleTime);
    return release;
}

static void testADSR_lin()
{
    for (float x = .8f; x > .01f; x *= .5f) {
        float r = measureRelease(x, true);
        printf("r(%f) = %f seconds ratio = %f\n", x, r, r / x);
    }
    assert(false);
}


static void testADRS4_1()
{
    // These values are just "known goods" from origina ADSR4.
    // But we want to preserve these for existing ADSR4 clients
    float r = measureRelease(.8f, false);
    assertClose(r, 2.2, .1);

    r = measureRelease(.4f, false);
    assertClose(r, .04, .01);

    r = measureRelease(.2f, false);
    assertClose(r, .006, .001);
}



void testADSR()
{
  //  testADRS4_1();
    testADSR_lin();
}