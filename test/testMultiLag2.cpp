
#include "Limiter.h"
#include "MultiLag2.h"
#include "simd.h"

#include "Analyzer.h"
#include "asserts.h"

template <class T>
static void _testMultiLag0()
{
    T l;
   simd_assertClose(l.get(), float_4(0), .0001);
}

static void testMultiLag0()
{
    _testMultiLag0<MultiLPF2>();
    _testMultiLag0<MultiLag2>();
}

// test that output eventually matches input
template <class T>
static void _testMultiLag1(T& dut)
{
    // test each float in the float_4
    for (int n = 0; n < 4; ++n) {

        float_4 input = 0;
        input[n] = 1;
        for (int i = 0; i < 10; ++i) {
            dut.step(input);
        }
        simd_assertClose(dut.get(), input, .0001)
    }
}

static void testMultiLag1()
{
    MultiLPF2 f;
    f.setCutoff(.4f);
    _testMultiLag1(f);

    MultiLag2 l;
    l.setAttack(.4f);
    l.setRelease(.4f);
    _testMultiLag1(l);
}

const float sampleRate = 44100;

// return first = fc, second = slope
template<typename T>
static std::pair<T, T> getLowpassStats(std::function<float(float)> filter, T FcExpected)
{
    const int numSamples = 16 * 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    auto x = Analyzer::getMaxAndShoulders(response, -3);

    const T cutoff = (T) FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples);

    // Is the peak at zero? i.e. no ripple.
    if (std::get<1>(x) == 0) {
        assertEQ(std::get<0>(x), -1);   // no LF shoulder
    } else {
        // Some higher order filters have a tinny bit of ripple
        float peakMag = std::abs(response.get(std::get<1>(x)));
        float zeroMag = std::abs(response.get(0));
        assertClose(peakMag / zeroMag, 1, .0001);
    }

    T slope = Analyzer::getSlopeLowpass(response, (float) FcExpected * 2, sampleRate);
    return std::make_pair(cutoff, slope);
}

template<typename T>
static void doLowpassTest(std::function<float(float)> filter, T Fc, T expectedSlope)
{
    auto stats = getLowpassStats<T>(filter, Fc);
    assertClose(stats.first, Fc, 3);    // 3db down at Fc

    //double slope = Analyzer::getSlope(response, (float) Fc * 2, sampleRate);
    assertClose(stats.second, expectedSlope, 1);          // to get accurate we need to go to higher freq, etc... this is fine
}

template <class T>
static void _testMultiLag2(T& dut, float f)
{
    for (int n = 0; n < 4; ++n) {
       // float input[100] = {0};
        float_4 input;
        std::function<float(float)> filter = [&input, n, &dut](float x) {
            input[n] = x;
            dut.step(input);
            auto y = dut.get()[n];
            return float(y);
        };
        doLowpassTest<float>(filter, f, -6);
    }
}

static void testMultiLag2int()
{
    MultiLPF2 f;
    float fC = 10.f;
    f.setCutoff(fC / sampleRate);
    _testMultiLag2(f, fC);

    MultiLag2 l;
    l.setAttack(fC / sampleRate);
    l.setRelease(fC / sampleRate);
    _testMultiLag2(l, fC);
}

static void testMultiLagDisable()
{
    MultiLag2 f;
    float fC = 10.f;
    f.setAttack(fC / sampleRate);
    f.setRelease(fC / sampleRate);

    // when enabled, should lag
   // const float buffer[8] = {1,1,1,1,1,1,1,1};
    const float_4 buffer(1);
    f.step(buffer);
    for (int i = 0; i < 4; ++i) {
        assertNE(f.get()[0], 1);
    }

    f.setEnable(false);
    f.step(buffer);
    for (int i = 0; i < 8; ++i) {
        assertEQ(f.get()[0], 1);
    }
}

// Let's test limiter here, too
static void testLimiter0()
{
    Limiter l;
    for (int i=0; i<100; ++i) {
        float_4 x = l.step(0);
        simd_assertEQ(x, float_4(0));
    }
}

static void testLimiterDC(float dc, float expectedDC)
{
    Limiter l;
    l.setTimes(1, 100, 1.f / 44100.f);
    float_4 input(dc);
    float_4 output(0);
    for (int i=0; i<100; ++i) {
        output = l.step(input);
    }
    simd_assertClose(output, float_4(expectedDC), .001);
}

static void testLimiterDC()
{
    testLimiterDC(0, 0);
    testLimiterDC(1, 1);
    testLimiterDC(10, 5);
}

static void testLimiterAC()
{
    Limiter l;
    l.setTimes(1, 100, 1.f / 44100.f);
    float_4 output(0);
    for (int i=0; i<100; ++i) {

        bool b = i & 1;
        float_4 input( b ? 10 : -10);
        output = l.step(input);
    }
    simd_assertClose(output, float_4(5), .001);
}

static void testLimiterAttack()
{
    Limiter l;
    l.setTimes(1, 100, 1.f / 44100.f);
    float_4 output = l.step(10.f);
    simd_assertGT(output, float_4(7));
}

void testMultiLag2()
{
 //   testLowpassLookup();
 //   testLowpassLookup2();
 //   testDirectLookup();
  //  testDirectLookup2();

    testMultiLag0();
    testMultiLag1();
    testMultiLag2int();
    testMultiLagDisable();

    testLimiter0();
    testLimiterDC();
    testLimiterAC();
    testLimiterAttack();
}