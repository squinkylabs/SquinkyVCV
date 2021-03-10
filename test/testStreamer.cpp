
#include "CubicInterpolator.h"
#include "FixedPointAccumulator.h"
#include "SqLog.h"
#include "Streamer.h"
#include "TestSignal.h"
#include "asserts.h"

static void testCubicInterp() {
    float data[] = {10, 9, 8, 7};

    // would need sample at -1 to interpolate sample 0
    assertEQ(CubicInterpolator<float>::canInterpolate(0, 4), false);
    assertEQ(CubicInterpolator<float>::canInterpolate(1, 4), true);
    assertEQ(CubicInterpolator<float>::canInterpolate(2, 4), false);
    assertEQ(CubicInterpolator<float>::canInterpolate(3, 4), false);

    assertEQ(CubicInterpolator<float>::canInterpolate(1.5f, 4), true);
    assertEQ(CubicInterpolator<float>::canInterpolate(1.99f, 4), true);

    float x = CubicInterpolator<float>::interpolate(data, 1);
    assertClose(x, 9, .00001);
    x = CubicInterpolator<float>::interpolate(data, 1.999f);
    assertClose(x, 8, .002);

    x = CubicInterpolator<float>::interpolate(data, 1.5f);
    assertClose(x, 8.5f, .0001);
}

static void testStream() {
    Streamer s;
    s._assertValid();
    assert(!s.canPlay(0));
    assert(!s.canPlay(1));
    assert(!s.canPlay(2));
    assert(!s.canPlay(3));
    s.step(0, false);
    s._assertValid();

    float x[6] = {0};
    s.setSample(0, x, 6);
    assert(s.canPlay(0));
    assert(!s.canPlay(1));
    s._assertValid();
}

static void testStreamEnd() {
    Streamer s;
    const int channel = 2;
    assert(!s.canPlay(channel));

    float x[6] = {0};
    s.setSample(channel, x, 6);
    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        s.step(0, false);
        s._assertValid();
    }
    assert(!s.canPlay(channel));
}

static void testStreamValues() {
    Streamer s;
    const int channel = 1;
    assert(!s.canPlay(channel));

    float x[6] = {.6f, .5f, .4f, .3f, .2f, .1f};
    assertEQ(x[0], .6f);

    s.setSample(channel, x, 6);
    s.setTranspose(float_4(1));
    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        float_4 v = s.step(0, false);
        assertClosePct(v[channel], .1f * (6 - i), 1);
        s._assertValid();
    }
    assert(!s.canPlay(channel));
}

static void testStreamXpose1() {
    Streamer s;
    const int channel = 3;
    assert(!s.canPlay(channel));

    float x[6] = {.6f, .5f, .4f, .3f, .2f, .1f};
    assertEQ(x[0], .6f);

    s.setSample(channel, x, 6);
    s.setTranspose(float_4(1));
    assert(s.canPlay(channel));
    s._assertValid();
    s.step(0, false);
    s._assertValid();
    assert(s.canPlay(channel));
}

// Now that we have cubic interpolation, this test no longer works.
// Need better ones.
static void testStreamXpose2() {
    Streamer s;
    const int channel = 3;
    assert(!s.canPlay(channel));

    float x[7] = {6, 5, 4, 3, 2, 1, 0};
    assertEQ(x[0], 6);

    s.setSample(channel, x, 7);
    s.setTranspose(float_4(2));
    assert(s.canPlay(channel));
    for (int i = 0; i < 3; ++i) {
        float_4 v = s.step(0, false);
        // start with 5, as interpoator forces us to start on second sample
        printf("i = %d v=%f\n", i, v[channel]);
        assertEQ(v[channel], 5 - (2 * i));
    }
    assert(!s.canPlay(channel));
}

static void testStreamRetrigger() {
    printf("testStreamRetrigger\n");
    Streamer s;
    const int channel = 0;

    float x[6] = {.6f, .5f, .4f, .3f, .2f, .1f};

    s.setSample(channel, x, 6);
    s.setTranspose(float_4(1));
    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        float_4 v = s.step(0, false);
        s._assertValid();
    }
    assert(!s.canPlay(channel));

    s.setSample(channel, x, 6);
    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        assert(s.canPlay(channel));
        s._assertValid();
        float_4 v = s.step(0, false);
        s._assertValid();
    }
    assert(!s.canPlay(channel));
}

static void testBugCaseHighFreq() {
    Streamer s;
    const int channel = 0;
    assert(!s.canPlay(channel));

    float x[6] = {0};
    s.setSample(channel, x, 6);
    s.setTranspose(float_4(33.4f));

    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        s.step(0, false);
        s._assertValid();
    }
    assert(!s.canPlay(channel));
}

#if 0
static bool testClick(float trans, bool injectChange=false, float signalValueAtChange = 0, float changeAmount = 0)
{
    SQINFO("------- tests click %f", trans);
    if (injectChange) {
        SQINFO("--- will inject change of %f at %f", changeAmount, signalValueAtChange);
    }
    const size_t size = 100000;
    float buffer[size];
    TestSignal<float>::generateSin(buffer, size, .001f);
    Streamer s;
    s.setSample(0, buffer, size);
    s.setTranspose(trans);
    s.setGain(0, 1.f);

    float lastSample = s.step(0, false)[0];
    float biggestChange = 0;
    bool haveInjected = false;

    for (int i=0; i< (size * .9f); ++i) {
        float_4 samples = s.step(0, false);
        float sample = samples[0];
        float change = std::abs(sample - lastSample);
        if (change > biggestChange) {
            SQINFO("change = %f at sample%d\n", change, i);
            biggestChange = change;
        }
        lastSample = sample;

        if (injectChange && !haveInjected && (i > size/2)) {
            float distToTarget = std::abs(lastSample - signalValueAtChange);
            if (distToTarget < .001) {
                SQINFO("injecting change at %f", lastSample);
                s.setTranspose(trans + changeAmount);
                haveInjected = true;
            }
        }
    }
    return haveInjected;
}


static void testClick2(float baseFreq, float trans,float changeAmount)
{
    SQINFO("------- tests click2 %f", trans);

    const size_t size = 100000;
    float buffer[size];
    TestSignal<float>::generateSin(buffer, size, baseFreq);
    Streamer s;
    s.setSample(0, buffer, size);
    s.setTranspose(trans);
    s.setGain(0, 1.f);

    float lastSample = s.step(0, false)[0];
    float biggestChange = 0;

    assert(trans < 1);
    assert(trans + changeAmount > 0);
    float counter = 0;
    bool sign = false;


    for (int i=0; i< (size * .9f); ++i) {
        float_4 samples = s.step(0, false);
        float sample = samples[0];
        float change = std::abs(sample - lastSample);
        if (change > biggestChange) {
            SQINFO("change = %f at sample%d\n", change, i);
            biggestChange = change;
        }
        lastSample = sample;

        if (++counter > 1000) {
                counter = 0;
                if (sign) {
                    s.setTranspose(trans + changeAmount);
                    sign = false;
                } else {
                    s.setTranspose(trans - changeAmount);
                    sign = true;
                }
        }


    }
}

static void testClick() {
#if 0
    bool b = testClick(1.0f);
    assert(!b);
    b = testClick(1.01f);
    assert(!b);
    b = testClick(.99f);
    assert(!b);

    b=testClick(.99f, true, 0, .03f);
    assert(b);
    b = testClick(.99f, true, .5, .03f);
    assert(b);
    b = testClick(1.02f, true, 0, -.04f);
    assert(b);
    b = testClick(1.01f, true, .5, -.04f);
    assert(b);
#endif

    testClick2(.001f, .99f, .04f );
    testClick2(.0001f, .986f, .0216f);
    testClick2(.01f, .993f, .01f);

    assert(false);
}
#endif

static void testFixedPoint0() {
    FixedPointAccumulator a;
    a.add(1);
    a.add(2);
    assertClose(a.getAsDouble(), 3, .00000001);
}

static void testFixedPoint1() {
    FixedPointAccumulator a;
    a.add(1);
    a.add(.999999999);
    assertClose(a.getAsDouble(), 2, .00000001);
}

static void testFixedPoint2() {
    FixedPointAccumulator a;
    a.add(1);
    a.add(1.000000001);
    assertClose(a.getAsDouble(), 2, .00000001);
}

static void testFixedPoint() {
    testFixedPoint0();
    testFixedPoint1();
    testFixedPoint2();

}

void testStreamer() {
    testCubicInterp();

    testStream();
    testStreamEnd();

    // since no-tranpose mode disable, this test doesn't work
   // testStreamValues();
  //  testStreamRetrigger();

    testStreamXpose1();
    // printf("fix testStreamXpose2\n");
    //testStreamXpose2();

    testBugCaseHighFreq();
    //testClick();
    testFixedPoint();
}