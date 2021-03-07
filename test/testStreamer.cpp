
#include "CubicInterpolator.h"
#include "Streamer.h"
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
    s.setTranspose(channel, false, 1.f);
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
    s.setTranspose(channel, true, 1.f);
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
    s.setTranspose(channel, true, 2.f);
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
    s.setTranspose(channel, false, 1.f);
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
    s.setTranspose(0, true, 33.4f);

    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        s.step(0, false);
        s._assertValid();
    }
    assert(!s.canPlay(channel));
}

void testStreamer() {
    testCubicInterp();

    testStream();
    testStreamEnd();
    testStreamValues();
    testStreamRetrigger();
    testStreamXpose1();
    // printf("fix testStreamXpose2\n");
    //testStreamXpose2();

    testBugCaseHighFreq();
}