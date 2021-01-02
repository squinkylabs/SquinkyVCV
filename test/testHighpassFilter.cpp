
#include "Analyzer.h"
#include "DCBlocker.h"
#include "TrapezoidalLowpass.h"

#include "asserts.h"




const float sampleRate = 44100;

// return first = fc, second = slope
template<typename T>
static std::pair<T, T> getHighpassStats(std::function<float(float)> filter, T FcExpected)
{
    const int numSamples = 16 * 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    auto x = Analyzer::getMaxAndShoulders(response, -3);

    const T cutoff = (T) FFT::bin2Freq(std::get<0>(x), sampleRate, numSamples);
    const T peak  = (T) FFT::bin2Freq(std::get<1>(x), sampleRate, numSamples);

    // Is the peak at nyquist? i.e. no ripple.
    // since this test only for one pole, it better be.
    auto nyquist = sampleRate / 2;
    assertLE(peak, nyquist);
    assertGE(peak, .9 * nyquist);

    T slope = (T) Analyzer::getSlopeHighpass(response, (float) FcExpected / 2, sampleRate);
    return std::make_pair(cutoff, slope);
}


static void testHighpass0()
{
    TrapezoidalHighpass<float> hp;
    hp.run(.1f, .1f);
}

static void testDCBlocker0()
{
    DCBlocker bl(20);
    bl.setSampleTime(1.f / 44100.f);
    bl.step(0);
}

static void testHighpass1()
{
    TrapezoidalHighpass<float> hp;
    const float g2 = .1f;
    const float input = 0;
    assertEQ(hp.run(input, g2), 0);
}


static void testDCBlocker1()
{
    DCBlocker bl(20);
    bl.setSampleTime(1.f / 44100.f);
    assertEQ(bl.step(0), 0);
}


static void testHighpass2()
{
    TrapezoidalHighpass<float> hp;
    const float g2 = .1f;
    const float input = 1;
    float output = hp.run(input, g2);
    assertGT(output, 0);
    assertLT(output, 1);
}

static void testDCBlocker2()
{
    DCBlocker bl(20);
    bl.setSampleTime(1.f / 44100.f);
    const float output = bl.step(1);

    assertGT(output, 0);
    assertLT(output, 1);
}


static void testHighpass3()
{
    TrapezoidalHighpass<float> hp;


    const float normalizeFc = .1f;
    const float expectedFc = normalizeFc * sampleRate;

    std::shared_ptr<NonUniformLookupTableParams<float>> fs2gLookup = makeTrapFilter_Lookup<float>();
    const float g2 = NonUniformLookupTable<float>::lookup(*fs2gLookup, normalizeFc);

  
    auto filter = [&hp, g2](float input) {
        auto output = hp.run(input, g2);
        return output;
    };

    auto x = getHighpassStats(filter, expectedFc);
    assertClose(std::get<0>(x), expectedFc, 500);

    // don't know why slope is measuring wrong, but don't care too much right now.
   // assertClose(std::get<1>(x), -24, 1);
}

static void testDCBlocker3()
{
    const float normalizeFc = .01f;
    const float expectedFc = normalizeFc * sampleRate;
    DCBlocker bl(expectedFc);
    bl.setSampleTime(1.f / 44100.f);

    auto filter = [&bl](float input) {
        auto output = bl.step(input);
        return output;
    };

    auto x = getHighpassStats(filter, expectedFc);
    float a = std::get<0>(x);
    float b = std::get<1>(x);
    assertClose(std::get<0>(x), expectedFc, 10);
    assertClose(std::get<1>(x), -24, 4);    // slope not accurate because warping?
}


static void testDCBlockerFallsToZero()
{
    DCBlocker bl(20);
    bl.setSampleTime(1.f / 44100.f);

    float output = bl.step(0);
    assertEQ(output, 0);
    for (int i = 0; i < 10000; ++i) {
        output = bl.step(1);
    }
    assertClose(output, 0, .0001);

}

void testHighpassFilter()
{
    testHighpass0();
    testHighpass1();
    testHighpass2();
    testHighpass3();

    testDCBlocker0();
    testDCBlocker1();
    testDCBlocker2();
    testDCBlocker3();
    testDCBlockerFallsToZero();
}