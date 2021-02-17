
#include <algorithm>

#include "Analyzer.h"
#include "Compressor.h"
#include "Compressor2.h"
#include "Limiter.h"
#include "MultiLag2.h"
#include "TestComposite.h"
#include "asserts.h"
#include "simd.h"
#include "tutil.h"

template <class T>
static void _testMultiLag0() {
    T l;
    simd_assertClose(l.get(), float_4(0), .0001);
}

static void testMultiLag0() {
    _testMultiLag0<MultiLPF2>();
    _testMultiLag0<MultiLag2>();
}

// test that output eventually matches input
template <class T>
static void _testMultiLag1(T& dut) {
    // test each float in the float_4
    for (int n = 0; n < 4; ++n) {
        float_4 input = 0;
        input[n] = 1;
        for (int i = 0; i < 10; ++i) {
            dut.step(input);
        }
        simd_assertClose(dut.get(), input, .0001);
    }
}

static void testMultiLag1() {
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
template <typename T>
static std::pair<T, T> getLowpassStats(std::function<float(float)> filter, T FcExpected) {
    const int numSamples = 16 * 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    auto x = Analyzer::getMaxAndShoulders(response, -3);

    const T cutoff = (T)FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples);

    // Is the peak at zero? i.e. no ripple.
    if (std::get<1>(x) == 0) {
        assertEQ(std::get<0>(x), -1);  // no LF shoulder
    } else {
        // Some higher order filters have a tinny bit of ripple
        float peakMag = std::abs(response.get(std::get<1>(x)));
        float zeroMag = std::abs(response.get(0));
        assertClose(peakMag / zeroMag, 1, .0001);
    }

    T slope = Analyzer::getSlopeLowpass(response, (float)FcExpected * 2, sampleRate);
    return std::make_pair(cutoff, slope);
}

template <typename T>
static void doLowpassTest(std::function<float(float)> filter, T Fc, T expectedSlope) {
    auto stats = getLowpassStats<T>(filter, Fc);
    assertClose(stats.first, Fc, 3);              // 3db down at Fc
    assertClose(stats.second, expectedSlope, 1);  // to get accurate we need to go to higher freq, etc... this is fine
}

template <class T>
static void _testMultiLag2(T& dut, float f) {
    for (int stage = 0; stage < 4; ++stage) {
        float_4 input = float_4(0);
        std::function<float(float)> filter = [&input, stage, &dut](float x) {
            input[stage] = x;
            dut.step(input);
            auto y = dut.get()[stage];
            return float(y);
        };
        doLowpassTest<float>(filter, f, -6);
    }
}

static void testMultiLag2int() {
    MultiLPF2 f;
    float fC = 10.f;
    f.setCutoff(fC / sampleRate);
    _testMultiLag2(f, fC);

    MultiLag2 l;
    l.setAttack(fC / sampleRate);
    l.setRelease(fC / sampleRate);
    _testMultiLag2(l, fC);
}

static void testMultiLagDisable() {
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
static void testLimiter0() {
    Limiter l;
    for (int i = 0; i < 100; ++i) {
        float_4 x = l.step(0);
        simd_assertEQ(x, float_4(0));
    }
}

static void testLimiterDC(float dc, float expectedDC) {
    Limiter l;
    const float a = .1f;
    l.setTimes(a, 100, 1.f / 44100.f);
    float_4 input(dc);
    float_4 output(0);
    for (int i = 0; i < 100; ++i) {
        output = l.step(input);
    }
    simd_assertClose(output, float_4(expectedDC), .001);
}

static void testLimiterDC() {
    testLimiterDC(0, 0);
    testLimiterDC(1, 1);
    testLimiterDC(10, 5);
}

static void testLimiterAC() {
    Limiter l;
    l.setTimes(.1f, 100, 1.f / 44100.f);
    float_4 output(0);
    for (int i = 0; i < 100; ++i) {
        bool b = i & 1;
        float_4 input(b ? 10.f : -10.f);
        output = l.step(input);
    }
    simd_assertClose(output, float_4(5), .001);
}

static void testLimiterAttack() {
    // test the it can go over the threshold during attack phase
    Limiter l;
    l.setTimes(1, 100, 1.f / 44100.f);
    float_4 output = l.step(10.f);
    simd_assertGT(output, float_4(7));
}

static void testLimiterAttackTC(float a) {
    float sampleRate = 44100.f;
    Limiter l;
    l.setTimes(a, 1000, 1.f / sampleRate);

    // const float aTarget = 10.f / AudioMath::E;
    const float aTarget = 10.f * float(1 - (1.f / AudioMath::E));

    const float aSec = float(a / 1000.0);
    const float aSamplesExpected = sampleRate * aSec;

    int samples = 0;
    for (bool done = false; !done; ++samples) {
        l.step(10.f);
        float_4 mem = l._lag()._memory();
        if (mem[0] > aTarget) {
            done = true;
            // let's shoot for within 1%
            const float limits = std::max(2.f, float(aSamplesExpected * .01));
            assertClose(samples, aSamplesExpected, limits);
        }
    }
}

static void testLimiterReleaseTC(float r) {
    float sampleRate = 44100.f;
    Limiter l;
    l.setTimes(.1f, r, 1.f / sampleRate);

    // first get to 10
    int x = 0;
    for (bool done = false; !done; ++x) {
        l.step(10.f);
        float_4 mem = l._lag()._memory();
        if (mem[0] > 9.9) {
            done = true;
        }
        if (x > 100000) {
            printf("after 100k, mem = %f\n", mem[0]);
            fflush(stdout);
            assert(false);
        }
    }

    // const float aTarget = 10.f / AudioMath::E;
    const float rTarget = 10.f * float((1.f / AudioMath::E));

    const float rSec = float(r / 1000.0);
    const float rSamplesExpected = sampleRate * rSec;

    int samples = 0;
    for (bool done = false; !done; ++samples) {
        l.step(0.f);
        float_4 mem = l._lag()._memory();
        if (mem[0] < rTarget) {
            done = true;
            // let's shoot for within 1%
            const float limits = rSamplesExpected * .01f;
            assertClose(samples, rSamplesExpected, limits);
        }
        if (samples > 1000000) {
            printf("after 3 100k, mem = %f\n", mem[0]);
            fflush(stdout);
            assert(false);
        }
    }
}

static void testLimiterTC(float a, float r) {
    testLimiterAttackTC(a);
    testLimiterReleaseTC(r);
}

static void testLimiterTC() {
    testLimiterTC(10, 1000);
    testLimiterTC(1, 100);
    testLimiterTC(.1f, 10000);
}

static void testLimiterPolyL() {
    using Comp = Compressor<TestComposite>;
    testPolyChannels<Comp>(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, 16);
}

static void testLimiterPolyR() {
    using Comp = Compressor<TestComposite>;
    testPolyChannels<Comp>(Comp::RAUDIO_INPUT, Comp::RAUDIO_OUTPUT, 16);
}

static void testCompUI() {
    using Comp = Compressor<TestComposite>;
    auto r = Comp::ratios();
    assert(r.size() == size_t(Cmprsr::Ratios::NUM_RATIOS));

    Cmprsr c;
    for (int i = 0; i < int(Cmprsr::Ratios::NUM_RATIOS); ++i) {
        c.setCurve(Cmprsr::Ratios(i));
        c.step(0);
    }
}

static void testCompLim(int inputId, int outputId) {
    using Comp = Compressor<TestComposite>;
    std::shared_ptr<Comp> comp = std::make_shared<Comp>();
    initComposite(*comp);

    comp->params[Comp::RATIO_PARAM].value = float(int(Cmprsr::Ratios::HardLimit));
    comp->params[Comp::THRESHOLD_PARAM].value = .1f;
    const double threshV = Comp::getSlowThresholdFunction()(.1);
    //printf("th .1 give %f volts\n", threshV);

    comp->inputs[inputId].channels = 1;
    comp->outputs[outputId].channels = 1;

    // at threshold, should get thresh out.
    comp->inputs[inputId].setVoltage(float(threshV), 0);
    TestComposite::ProcessArgs args;
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }

    float output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    comp->inputs[inputId].setVoltage(float(threshV), 0);
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    assertGT(4, threshV);
    comp->inputs[inputId].setVoltage(4, 0);
    for (int i = 0; i < 2000; ++i) {
        comp->process(args);
    }
    output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    comp->inputs[inputId].setVoltage(10, 0);
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    comp->inputs[inputId].setVoltage(0, 0);
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    output = comp->outputs[outputId].voltages[0];
    assertClose(output, 0, .001);
}

static void testCompLim() {
    using Comp = Compressor<TestComposite>;
    testCompLim(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT);
    testCompLim(Comp::RAUDIO_INPUT, Comp::RAUDIO_OUTPUT);
}

static void testLagZeroAttack(bool isZero) {
    float sampleRate = 44100;
    float sampleTime = 1.f / sampleRate;
    MultiLag2 lag;

    lag.setAttack(.01f);
    lag.setRelease(.01f);
    lag.setInstantAttack(isZero);

    // when input is increasing, should follow input
    float_4 in(1.34f);

    lag.step(in);
    auto out = lag.get();
    auto peak = out;

    if (isZero) {
        simd_assertEQ(out, in);
    } else {
        simd_assertLT(out, in);
    }

    in = float_4(1);
    lag.step(in);
    out = lag.get();

    if (isZero) {
        simd_assertGT(out, in);
    }
}

static void testLagZeroAttack() {
    testLagZeroAttack(true);
    testLagZeroAttack(false);
}

static void testCompRatio(int inputId, int outputId, Cmprsr::Ratios ratio) {
    using Comp = Compressor<TestComposite>;
    std::shared_ptr<Comp> comp = std::make_shared<Comp>();
    initComposite(*comp);

    comp->params[Comp::RATIO_PARAM].value = float(int(ratio));
    comp->params[Comp::THRESHOLD_PARAM].value = .1f;
    const double threshV = Comp::getSlowThresholdFunction()(.1);

    comp->inputs[inputId].channels = 1;
    comp->outputs[outputId].channels = 1;

    // at threshold, should get thresh out.
    comp->inputs[inputId].setVoltage(float(threshV), 0);
    TestComposite::ProcessArgs args;
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }

    float output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    const float threshDb = float(AudioMath::db(threshV));

    float expectedRatio = 0;
    switch (ratio) {
        case Cmprsr::Ratios::_2_1_hard:
            expectedRatio = 2;
            break;
        case Cmprsr::Ratios::_4_1_hard:
            expectedRatio = 4;
            break;
        case Cmprsr::Ratios::_8_1_hard:
            expectedRatio = 8;
            break;
        case Cmprsr::Ratios::_20_1_hard:
            expectedRatio = 20;
            break;
        default:
            assert(false);
    }

    for (int mult = 2; (mult * threshV) < 10; mult *= 2) {
        float input = float(threshV * mult);
        const float inputDb = float(AudioMath::db(input));
        comp->inputs[inputId].setVoltage(input, 0);
        for (int i = 0; i < 2000; ++i) {
            comp->process(args);
        }
        output = comp->outputs[outputId].voltages[0];
        float outputDb = float(AudioMath::db(output));

        const float observedRatio = (inputDb - threshDb) / (outputDb - threshDb);
        assertClosePct(observedRatio, expectedRatio, 15);
    }
}

static void testCompRatio8() {
    using Comp = Compressor<TestComposite>;
    testCompRatio(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, Cmprsr::Ratios::_8_1_hard);
    testCompRatio(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, Cmprsr::Ratios::_4_1_hard);
    testCompRatio(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, Cmprsr::Ratios::_20_1_hard);
    testCompRatio(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, Cmprsr::Ratios::_2_1_hard);
}

template <class T>
class TestBothComp {
public:
    TestBothComp() {
        comp_ = std::make_shared<T>();
        initComposite(*comp_);
    }

    void testPoly() {
        // initial run with 1 channel
        setNumChan(1);
        setInputs(1, 100.f);
        run(1000);
        const float x = comp_->outputs[T::LAUDIO_OUTPUT].voltages[0];

        assertLT(x, 50);
        assertGT(x, 1);

        // now patch more channels, see if comp recognizes the change
        setNumChan(4);
        setInputs(4, 100.f);
        run(1000);
        const float x0 = comp_->outputs[T::LAUDIO_OUTPUT].voltages[0];
        const float x1 = comp_->outputs[T::LAUDIO_OUTPUT].voltages[1];
        const float x2 = comp_->outputs[T::LAUDIO_OUTPUT].voltages[2];
        const float x3 = comp_->outputs[T::LAUDIO_OUTPUT].voltages[3];
        assertLT(x0, 50);
        assertLT(x1, 50);
        assertLT(x2, 50);
        assertLT(x3, 50);
        assertGT(x0, 1);
        assertGT(x1, 1);
        assertGT(x2, 1);
        assertGT(x3, 1);
    }

private:
    std::shared_ptr<T> comp_;
    TestComposite::ProcessArgs args;

    void setNumChan(int x) {
        comp_->inputs[T::LAUDIO_INPUT].channels = x;
        comp_->outputs[T::LAUDIO_OUTPUT].channels = x;
    }
    void run(int iterations) {
        for (int i = 0; i < iterations; ++i) {
            comp_->process(args);
        }
    }

    void setInputs(int num, float value) {
        for (int i = 0; i < num; ++i) {
            comp_->inputs[T::LAUDIO_INPUT].setVoltage(value, i);
        }
    }
};

static void testCompPoly() {
  

    TestBothComp<Compressor2<TestComposite>> test2;
    test2.testPoly();

    TestBothComp<Compressor<TestComposite>> test;
    test.testPoly();
}

static void testCompPolyOrig() {
    using Comp = Compressor<TestComposite>;
    std::shared_ptr<Comp> comp = std::make_shared<Comp>();
    initComposite(*comp);

    comp->inputs[Comp::LAUDIO_INPUT].channels = 1;
    comp->outputs[Comp::LAUDIO_OUTPUT].channels = 1;

    // huge input.
    comp->inputs[Comp::LAUDIO_INPUT].setVoltage(100, 0);
    TestComposite::ProcessArgs args;
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    float x = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[0];
    assertLT(x, 50);

    const int nchTest = 4;
    comp->inputs[Comp::LAUDIO_INPUT].channels = nchTest;
    comp->outputs[Comp::LAUDIO_OUTPUT].channels = nchTest;

    for (int i = 0; i < nchTest; ++i) {
        comp->inputs[Comp::LAUDIO_INPUT].setVoltage(100, i);
    }

    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    float x0 = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[0];
    float x1 = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[1];
    float x2 = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[2];
    float x3 = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[3];
    assertLT(x0, 50);
    assertLT(x1, 50);
    assertLT(x2, 50);
    assertLT(x3, 50);
}


static void testIndependentFc() {
    MultiLPF2 lpf;
    float_4 fc(.01f, .02f, .04f, .08f);

    lpf.setCutoffPoly(fc);
    lpf.step( float_4(1));
    float_4 x = lpf.get();

    printf("got %s\n", toStr(x).c_str());
    fflush(stdout);

    assertGT(x[1], x[0]);
    assertGT(x[2], x[1]);
    assertGT(x[3], x[2]);

    assertClosePct(x[1], x[0] * 2, 20);
    assertClosePct(x[2], x[1] * 2, 40);
    assertClosePct(x[3], x[2] * 2, 40);   
}

void testMultiLag2() {
    //   testLowpassLookup();
    //   testLowpassLookup2();
    //   testDirectLookup();
    //  testDirectLookup2();

#if 1
    testMultiLag0();
    testMultiLag1();
    testMultiLag2int();
    testMultiLagDisable();

    testLimiter0();
    testLimiterDC();
    testLimiterAC();
    testLimiterAttack();
    testLimiterTC();
    testLimiterPolyL();
    testLimiterPolyR();

    testCompUI();
    testCompLim();
#endif


    testLagZeroAttack();
    testCompRatio8();

    // testCompPolyOrig();
    testCompPoly();
    testIndependentFc();

}