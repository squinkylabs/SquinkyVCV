
#include "CubicInterpolator.h"
#include "FixedPointAccumulator.h"
#include "SqLog.h"
#include "Streamer.h"
#include "TestSignal.h"
#include "asserts.h"

//********************************* Interpolator tests ********************************

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

    const float offset = 1.5f;
    const unsigned int delayTimeSamples = CubicInterpolator<float>::getIntegerPart(offset);
    const float y0 = data[delayTimeSamples - 1];
    const float y1 = data[delayTimeSamples];
    const float y2 = data[delayTimeSamples + 1];
    const float y3 = data[delayTimeSamples + 2];
    x = CubicInterpolator<float>::interpolate(offset, y0, y1, y2, y3);
    assertClose(x, 8.5f, .0001);
}

static void testCubicInterpDouble() {
    double data[] = {10, 9, 8, 7};

    double x = CubicInterpolator<double>::interpolate(data, 1);
    assertClose(x, 9, .0000001);
    x = CubicInterpolator<double>::interpolate(data, 1.9999999);
    assertClose(x, 8, .0000001);

    x = CubicInterpolator<double>::interpolate(data, 1.5);
    assertClose(x, 8.5, .000001);
}

// more perverse data values
static void testCubicInterp2Double() {
    double data[] = {1, 1, 21, 21};

    double x = CubicInterpolator<double>::interpolate(data, 1);
    assertClose(x, 1, .0000001);
    x = CubicInterpolator<double>::interpolate(data, 1.9999999);
    assertClose(x, 21, .00001);

    x = CubicInterpolator<double>::interpolate(data, 1.5);
    assertClose(x, 11, .0000001);
}

// more perverse data values
static void testCubicInterp3Double() {
    double data[] = {0, 100, 100, 0};

    double x = CubicInterpolator<double>::interpolate(data, 1.5);
    assertClose(x, 100, 13);

    double data2[] = {100, 100, 100, 100};
    x = CubicInterpolator<double>::interpolate(data2, 1.5);
    assertClose(x, 100, .0000001);
    x = CubicInterpolator<double>::interpolate(data2, 1.25);
    assertClose(x, 100, .0000001);
    x = CubicInterpolator<double>::interpolate(data2, 1.75);
    assertClose(x, 100, .0000001);
}

//****************************************** Streamer tests *****************
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
    assert(!s.channels[0].loopActive);
}

static void testStreamLoopData() {
  
    Streamer s;
    s.setSample(3, nullptr, 1000);
    CompiledRegion::LoopData loopData;
    s.setLoopData(3, loopData);
    assert(!s.channels[3].loopActive);

    loopData.offset = 123;
    s.setLoopData(3, loopData);
    assert(s.channels[3].loopActive);
    assert(s.channels[3].loopData == loopData);

    loopData = CompiledRegion::LoopData();
    loopData.loop_start = 100;
    loopData.loop_end = 200;
    s.setSample(2, nullptr, 1000);
    s.setLoopData(2, loopData);
    assert(s.channels[2].loopActive);

    loopData = CompiledRegion::LoopData();
    loopData.loop_start = 300;
    loopData.loop_end = 200;
    s.setLoopData(2, loopData);
    assert(!s.channels[2].loopActive);
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
    assert(!s.channels[channel].loopActive);
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
    SQINFO("--- here we go");
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        float_4 v = s.step(0, false);
        SQINFO("sample[%d] = %f", i, v[channel]);
        assertClose(v[channel], x[i], .01);
        s._assertValid();
    }
    assert(!s.canPlay(channel));
    assert(!s.channels[channel].loopActive);
}

static void testStreamOffset() {
    Streamer s;
    const int channel = 1;
    assert(!s.canPlay(channel));

    float x[6] = {.6f, .5f, .4f, .3f, .2f, .1f};
    assertEQ(x[0], .6f);

    s.setSample(channel, x, 6);
    CompiledRegion::LoopData loopData;
    loopData.offset = 1;
    s.setLoopData(channel, loopData);
    assert(s.channels[channel].loopActive);
    s.setTranspose(float_4(1));
    assert(s.canPlay(channel));
    SQINFO("--- here we go");
    for (int i = 0; i < 5; ++i) {
        s._assertValid();
        float_4 v = s.step(0, false);
        SQINFO("sample[%d] = %f", i, v[channel]);
        assertClose(v[channel], x[i] - .1f, .01);
        s._assertValid();
    }
    assert(!s.canPlay(channel));
    assert(s.channels[channel].loopActive);

    SQWARN("write full tests for setLoopData");
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
    assert(!s.channels[channel].loopActive);
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
    assert(!s.channels[channel].loopActive);
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
    assert(!s.channels[channel].loopActive);
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

// this fixed point acc was a dead end....
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
    testCubicInterpDouble();
    testCubicInterp2Double();
    testCubicInterp3Double();

    testStream();
    testStreamLoopData();
    testStreamValues();
    testStreamEnd();
    testStreamXpose1();
    testBugCaseHighFreq();
    testStreamOffset();

    testFixedPoint();
}