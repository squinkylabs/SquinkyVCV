
#include "asserts.h"
#include "poly.h"
#include "Analyzer.h"
#include "SinOscillator.h"
#include "TestSignal.h"

static void test0()
{
    Poly<float, 11> poly;
    float x = poly.run(0);
    assertEQ(x, 0);

    x = poly.run(1);
    assertEQ(x, 0);

    poly.setGain(0, 1);
    x = poly.run(0);
    assertEQ(x, 0);
}

static void test1()
{
    Poly<float, 11> poly;
    poly.setGain(0, 1);
    float x = poly.run(1);
    assertNE(x, 0);
}

static void testPureTerm(int term)
{
    float desiredFreq = 500.0f;
    int numSamples = 16 * 1024;
    const float sampleRate = 44100.0f;
    double actualFreq = Analyzer::makeEvenPeriod(desiredFreq, sampleRate, numSamples);

    SinOscillatorParams<float> sinParams;
    SinOscillatorState<float> sinState;
    SinOscillator<float, false>::setFrequency(sinParams, float(actualFreq / sampleRate));

    Poly<double, 11> poly;
    poly.setGain(term, 1);

    FFTDataCpx spectrum(numSamples);
    Analyzer::getSpectrum(spectrum, false, [&sinState, &sinParams, &poly]() {
        float sin = SinOscillator<float, false>::run(sinState, sinParams);
        const float x = poly.run(sin);
        return x;
        });

    const int order = term + 1;
    Analyzer::assertSingleFreq(spectrum, float(actualFreq * order), sampleRate);
}

static void testTerms()
{
    for (int i = 0; i < 10; ++i) {
        testPureTerm(i);
    }
}


static void _testDC(int term, float gain)
{
    Poly<double, 11> poly;
    poly.setGain(term, 1);

    // Will generate sine at fs * .01 (around 400 Hz).
    SinOscillatorParams<float> sinp;
    SinOscillatorState<float> sins;
    SinOscillator<float, false>::setFrequency(sinp, .01f);

    // Run sin through Chebyshevs at specified gain
    auto func = [&sins, &sinp, &poly, gain]() {
        const float sin = gain * SinOscillator<float, false>::run(sins, sinp);
        const float x = poly.run(sin);
        return x;
    };

    const int bufferSize = 16 * 1024;
    float buffer[bufferSize];
    for (int i = 0; i < bufferSize; ++i) {
        buffer[i] = func();
    }
    double dc = TestSignal<float>::getDC(buffer, bufferSize);
    assertLT(dc, .001);


    printf("gain = %f term = %d, dc=%f\n", gain, term, dc);
}

static void testDC()
{
  //  _testDC(1, 1);
   // _testDC(2, 1);
  //  _testDC(1, .5);
  //  _testDC(2, .5);
    for (int i = 0; i < 10; ++i) {
        _testDC(i, .56f);
        _testDC(i, .83f);
        _testDC(i, .17f);
    }
}

void testPoly()
{
    test0();
    test1();
    testTerms();
    testDC();

}