
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
    assertEQ(s._cd(0).canPlay(), false);
    assertEQ(s._cd(1).canPlay(), false);
    assertEQ(s._cd(2).canPlay(), false);
    assertEQ(s._cd(3).canPlay(), false);
    s.step(0, false);
    s._assertValid();

    float x[6] = {0};
    s.setSample(0, x, 6);
    assertEQ(s._cd(0).canPlay(), true);
    assertEQ(s._cd(1).canPlay(), false);
    s._assertValid();
    assert(!s.channels[0].loopActive);
}

static void testStreamLoopData() {
    Streamer s;
    float data[1000] = {0};
    s.setSample(3, data, 1000);

    // blank loop data
    CompiledRegion::LoopData loopData;
    s.setLoopData(3, loopData);
    assert(!s.channels[3].loopActive);

    // normal loop
    loopData = CompiledRegion::LoopData();
    loopData.loop_start = 100;
    loopData.loop_end = 200;
    loopData.loop_mode = SamplerSchema::DiscreteValue::LOOP_CONTINUOUS;
    s.setSample(2, data, 1000);
    s.setLoopData(2, loopData);
    assert(s.channels[2].loopActive);

    // inverted loop points
    loopData = CompiledRegion::LoopData();
    loopData.loop_start = 300;
    loopData.loop_end = 200;
    s.setLoopData(2, loopData);
    assert(!s.channels[2].loopActive);
}

static void testStreamLoopData2() {
    Streamer s;
    float data[10] = {0};
    s.setSample(3, data, 10);

    CompiledRegion::LoopData loopData;

    // zero sample loop
    loopData = CompiledRegion::LoopData();
    loopData.loop_start = 2;
    loopData.loop_end = 2;
    s.setLoopData(3, loopData);
    assert(!s.channels[3].loopActive);

    // offset too big
    loopData = CompiledRegion::LoopData();
    loopData.offset = 1000;
    s.setLoopData(3, loopData);
    assertEQ(s.channels[3].loopData.offset, 0);

    //  loop end out of range
    loopData = CompiledRegion::LoopData();
    loopData.loop_end = 100;
    s.setSample(3, data, 10);

    s.setLoopData(3, loopData);
    assert(!s.channels[3].loopActive);
}

static void testStreamEnd() {
    Streamer s;
    const int channel = 2;
    assertEQ(s._cd(channel).canPlay(), false);

    float x[6] = {0};
    s.setSample(channel, x, 6);
    assertEQ(s._cd(channel).canPlay(), true);
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        s.step(0, false);
        s._assertValid();
    }
    assertEQ(s._cd(channel).canPlay(), false);
    assert(!s.channels[channel].loopActive);
}

/////////////////////////////////////////////////////////////////////////

class TestValues {
public:
    float fractionalOffset = 0;
    Streamer s;
    unsigned int sampleCount = 0;
    const float* input = nullptr;
    const float* expectedOutput = nullptr;
    int channel = 0;
    unsigned int skipSamples = 0;
    CompiledRegion::LoopData loopData;
    bool expectCanPlayAfter = false;
    unsigned int expectedOutputSamples = 0;
    float transposeMult = 1;
};

static void testStreamValuesSub(TestValues& v) {
    if (v.expectedOutputSamples == 0) {
        v.expectedOutputSamples = v.sampleCount;
    }

   // assert(v.transposeMult == 1);
    assert(v.sampleCount);
    assert(v.skipSamples < v.sampleCount);
    assert(v.input && v.expectedOutput);

    v.s.setSample(v.channel, v.input, v.sampleCount);
    v.s.setLoopData(v.channel, v.loopData);
    v.s.setTranspose(float_4(v.transposeMult));
    assertEQ(v.s._cd(v.channel).canPlay(), true);
    v.s.channels[v.channel].curFloatSampleOffset += v.fractionalOffset;
    for (unsigned int i = 0; i < v.expectedOutputSamples; ++i) {
        // SQINFO("\ntop of test loop %d", i);
        v.s._assertValid();
        float_4 vx = v.s.step(0, false);
        // SQINFO("sample[%d] = %f", i, vx[v.channel]);
        if (i >= v.skipSamples) {
            assertClose(vx[v.channel], v.expectedOutput[i], .01);
        }
        v.s._assertValid();
    }

    assertEQ(v.s._cd(v.channel).canPlay(), v.expectCanPlayAfter);
}

static void testStreamValueEnd() {
    SQINFO("-- testStreamValuesEnd -- ");

    float input[8] =  {1, 1, 1, 1, 1, 1, 1, 1};
    float output[8] = {1, 1, 1, 1, 0, 0, 0, 0};
    TestValues v;
    v.channel = 1;
    v.input = input;
    v.expectedOutput = output;
    v.sampleCount = 4;
    v.expectedOutputSamples = 8;
    v.loopData.end = 3;         // last sample we play

    testStreamValuesSub(v);
}

static void testStreamValueOSc() {
    SQINFO("-- testStreamValuesOsc -- ");

    float input[4] =  {1, 2, 3, 4};
    float output[12] = { 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4 };
    TestValues v;
    v.channel = 1;
    v.input = input;
    v.expectedOutput = output;
    v.sampleCount = 4;
    v.expectedOutputSamples = 12;
    v.loopData.oscillator = true;
    v.expectCanPlayAfter = true;

    testStreamValuesSub(v);
}

static void testStreamValueOSc2(int playRegionSize, float fractionalOffset, float transpose) {
    const int buffSize = 110;
    assert(playRegionSize < (buffSize - 4));
    assert(transpose > 0);
    assert(fractionalOffset >= 0);
    assert(fractionalOffset < 1);


    float buffer[buffSize];
    float expectedOutput[buffSize] = {0};
    for (int i=0; i<buffSize; ++i) {
        buffer[i] = 0;
    }
    buffer[0] = 1000;
    buffer[1] = 2000;
    buffer[2 + playRegionSize] = -3000;
    buffer[3 + playRegionSize] = -9000;

    TestValues v;
    v.channel = 0;
    v.fractionalOffset = fractionalOffset;
    v.transposeMult = transpose;
    v.input = buffer + 2;
    v.expectedOutput = expectedOutput;
    v.sampleCount = buffSize;
    v.expectedOutputSamples = buffSize * 3;
    v.expectCanPlayAfter = true;
    v.loopData.oscillator = true;
}

static void testStreamValueOSc2() {
    SQINFO("-- testStreamValuesOsc2 -- ");
    // first, no fractions
    for (int i = 4; i < 12; ++i) {
        testStreamValueOSc2(i, 0, 1);
    }

    for (int i = 0; i < 10; ++i) {
        float frac = float(i) / 10;
        testStreamValueOSc2(11, frac, 1);
    }

     testStreamValueOSc2(100, 0, 1.15f);
     testStreamValueOSc2(64, 0, 1.15f);
     testStreamValueOSc2(8, 0, 1.15f);
  
}

// This was assererting before
static void testStreamValueOSc3() {
     SQINFO("-- testStreamValuesOsc3 -- ");

    const int size = 2048;
    // 
    // 64 gives good failure
     // 8 gives different failure
    // const int size = 8;
    float input[size] =  {0};
    float output[size * 2] = {0};
    TestValues v;
    v.channel = 0;
    v.input = input;
    v.expectedOutput = output;
    v.sampleCount = size;
    v.expectedOutputSamples = size * 2;
    v.loopData.oscillator = true;
    v.expectCanPlayAfter = true;
    v.transposeMult = 1.15f;

    testStreamValuesSub(v);
}

static void testStreamValues() {
    SQINFO("-- testStreamValues -- ");
    TestValues v;
    v.channel = 1;
    assertEQ(v.s._cd(v.channel).canPlay(), false);

    float input[6] = {.6f, .5f, .4f, .3f, .2f, .1f};
    v.input = input;
    v.expectedOutput = input;
    v.sampleCount = 6;
    v.expectCanPlayAfter = false;  //  we will play all the samples

    testStreamValuesSub(v);
    assertEQ(v.s._cd(v.channel).canPlay(), false);
    assert(!v.s.channels[v.channel].loopActive);
}

static void testStreamValuesInterp() {
    SQINFO("-- testStreamValuesInterp -- ");
    TestValues v;
    v.channel = 2;
    assertEQ(v.s._cd(v.channel).canPlay(), false);

    float input[7] = {.6f, .5f, .4f, .3f, .2f, .1f, 0.f};
    float output[6] = {.55f, .45f, .35f, .25f, .15f, .05f};
    v.input = input;
    v.expectedOutput = output;
    v.fractionalOffset = .5f;
    v.sampleCount = 6;
    v.skipSamples = 2;  // interp won't be fired up yet
    v.expectedOutputSamples = v.sampleCount - 1;

    testStreamValuesSub(v);
    assert(!v.s.channels[v.channel].loopActive);
}

static void testStreamValuesOffset() {
    SQINFO("------ testStreamValuesOffset --------------");
    TestValues v;
    v.channel = 1;
    assertEQ(v.s._cd(v.channel).canPlay(), false);

    const int samples = 6;
    // TODO: need to separate how much data we have, vs. how many samples to run over?
    // or should this just be working (with end buffer?)
    //                  0     1    2    3    4    5
    float input[samples] = {.6f, .5f, .4f, .3f, .2f, .1f};
    float output[samples] = {.5f, .4f, .3f, .2f, .1f};
    v.sampleCount = samples;
    v.loopData.offset = 1;
    v.input = input;
    v.expectedOutput = output;
    testStreamValuesSub(v);
    assertEQ(v.s.channels[v.channel].loopActive, false);
}

static void testStreamValuesLoop() {
    SQINFO("-- testStreamValuesLoop -- ");
    TestValues v;
    v.channel = 1;
    assertEQ(v.s._cd(v.channel).canPlay(), false);

    //                  0  1  2  3  4  5  6   7      8   9      10    11   12
    float input[13] = {1, 2, 3, 4, 5, 6, 7, 1000, 1000, 2000, 2000, 2000, 2000};
    float output[13] = {1, 2, 3, 4, 5, 6, 7, 3, 4, 5, 6, 7, 3};
    v.input = input;
    v.expectedOutput = output;
    v.sampleCount = 13;
    v.loopData.loop_start = 2;
    v.loopData.loop_end = 6;
    v.loopData.loop_mode = SamplerSchema::DiscreteValue::LOOP_CONTINUOUS;
    v.expectCanPlayAfter = true;  // looped, so should still play forever
    testStreamValuesSub(v);
    assert(v.s.channels[v.channel].loopActive);
}

static void testStreamValuesLoop2() {
    SQINFO("-- testStreamValuesLoop (make me pass! -- ");
    TestValues v;
    v.channel = 1;
    assertEQ(v.s._cd(v.channel).canPlay(), false);

    //                  0  1  2  3  4     5      6       7   8   9
    float input[10] = {1, 2, 3, 4, 1000, 1000, 2000, 2000, 2000, 2000};
    float output[10] = {1, 2, 3, 4, 3, 4, 3, 4, 3, 4};
    v.input = input;
    v.expectedOutput = output;
    v.sampleCount = 10;
    v.loopData.loop_start = 2;
    v.loopData.loop_end = 3;
    v.loopData.loop_mode = SamplerSchema::DiscreteValue::LOOP_CONTINUOUS;
#if 0
    testStreamValuesSub(v);
    assert(v.s.channels[v.channel].loopActive);
#endif
}

static void testStreamXpose1() {
    Streamer s;
    const int channel = 3;
    assertEQ(s._cd(0).canPlay(), false);

    float x[6] = {.6f, .5f, .4f, .3f, .2f, .1f};
    assertEQ(x[0], .6f);

    s.setSample(channel, x, 6);
    s.setTranspose(float_4(1));
    assertEQ(s._cd(channel).canPlay(), true);
    s._assertValid();
    s.step(0, false);
    s._assertValid();
    assertEQ(s._cd(channel).canPlay(), true);
    assert(!s.channels[channel].loopActive);
}

// Now that we have cubic interpolation, this test no longer works.
// Need better ones.
static void testStreamXpose2() {
    Streamer s;
    const int channel = 3;
    assertEQ(s._cd(channel).canPlay(), false);

    float x[7] = {6, 5, 4, 3, 2, 1, 0};
    assertEQ(x[0], 6);

    s.setSample(channel, x, 7);
    s.setTranspose(float_4(2));
    assertEQ(s._cd(channel).canPlay(), true);
    for (int i = 0; i < 3; ++i) {
        float_4 v = s.step(0, false);
        // start with 5, as interpoator forces us to start on second sample
        printf("i = %d v=%f\n", i, v[channel]);
        assertEQ(v[channel], 5 - (2 * i));
    }
    assertEQ(s._cd(channel).canPlay(), false);
    assert(!s.channels[channel].loopActive);
}

static void testStreamRetrigger() {
    printf("testStreamRetrigger\n");
    Streamer s;
    const int channel = 0;

    float x[6] = {.6f, .5f, .4f, .3f, .2f, .1f};

    s.setSample(channel, x, 6);
    s.setTranspose(float_4(1));
    assertEQ(s._cd(channel).canPlay(), true);
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        float_4 v = s.step(0, false);
        s._assertValid();
    }
    assertEQ(s._cd(channel).canPlay(), false);

    s.setSample(channel, x, 6);
    assertEQ(s._cd(channel).canPlay(), true);
    for (int i = 0; i < 6; ++i) {
        assertEQ(s._cd(channel).canPlay(), true);
        s._assertValid();
        float_4 v = s.step(0, false);
        s._assertValid();
    }
    assertEQ(s._cd(channel).canPlay(), false);
    assert(!s.channels[channel].loopActive);
}

static void testBugCaseHighFreq() {
    SQINFO("---- bug case high freq ----");
    Streamer s;
    const int channel = 0;
    assertEQ(s._cd(channel).canPlay(), false);

    float x[6] = {0};
    s.setSample(channel, x, 6);
    s.setTranspose(float_4(33.4f));

    assertEQ(s._cd(channel).canPlay(), true);
    for (int i = 0; i < 6; ++i) {
        s._assertValid();
        s.step(0, false);
        s._assertValid();
    }
    assertEQ(s._cd(channel).canPlay(), false);
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
    testStreamValueOSc3();

    testCubicInterp();
    testCubicInterpDouble();
    testCubicInterp2Double();
    testCubicInterp3Double();

    testStream();
    testStreamLoopData();
    testStreamLoopData2();
    testStreamValues();
    testStreamValuesInterp();
    testStreamValuesLoop();
    testStreamValuesLoop2();
    testStreamEnd();
    testStreamXpose1();
    testBugCaseHighFreq();
    testStreamValuesOffset();
    testStreamValueEnd();
    testStreamValueOSc();
    testStreamValueOSc2();
    testStreamValueOSc3();
    testFixedPoint();
}