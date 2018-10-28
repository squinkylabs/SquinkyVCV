
#include "asserts.h"
#include "poly.h"
#include "Analyzer.h"
#include "SinOscillator.h"
#include "TestSignal.h"

static void test0()
{
    Poly<float, 10> poly;
    float x = poly.run(0, 1);
    assertEQ(x, 0);

    x = poly.run(1, 1);
    assertEQ(x, 0);

    poly.setGain(0, 1);
    x = poly.run(0, 1);
    assertEQ(x, 0);
}

static void test1()
{
    Poly<float, 10> poly;
    poly.setGain(0, 1);
    float x = poly.run(1, 0);
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

    Poly<double, 10> poly;
    poly.setGain(term, 1);

    FFTDataCpx spectrum(numSamples);
    Analyzer::getSpectrum(spectrum, false, [&sinState, &sinParams, &poly]() {
        float sin = SinOscillator<float, false>::run(sinState, sinParams);
        const float x = poly.run(sin, 1);
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


static void temp()
{
    Poly<double, 10> poly;
    poly.run(0, 1);
    poly._dumpDC();
    
    assertEQ(poly.getDC(0), 1);
    assertEQ(poly.getDC(1), -1);
    assertEQ(poly.getDC(2), 1);
    assertEQ(poly.getDC(3), -1);
    assertEQ(poly.getDC(4), 1);
}

static void _testDC(int term, float gain)
{
    Poly<double, 10> poly;
    poly.setGain(term, 1);

    // Will generate sine at fs * .01 (around 400 Hz).
    SinOscillatorParams<float> sinp;
    SinOscillatorState<float> sins;
    SinOscillator<float, false>::setFrequency(sinp, .01f);

    // Run sin through Chebyshevs at specified gain
    auto func = [&sins, &sinp, &poly, gain]() {
        const float sin = gain * SinOscillator<float, false>::run(sins, sinp);
        const float x = poly.run(sin, gain);
        return x;
    };

    const int bufferSize = 16 * 1024;
    float buffer[bufferSize];
    for (int i = 0; i < bufferSize; ++i) {
        buffer[i] = func();
    }
    double dc = TestSignal<float>::getDC(buffer, bufferSize);
   
    printf("in test gain = %f term = %d, dc=%f\n", gain, term, dc);
    poly._dumpDC();
    assertClose(dc, 0, .001);
}

static void testDC()
{
  //  _testDC(1, 1);
   // _testDC(2, 1);
  //  _testDC(1, .5);
  //  _testDC(2, .5);
    for (int i = 0; i < 10; ++i) {
        _testDC(i, 1);
    }
    for (int i = 0; i < 10; ++i) {
        _testDC(i, .56f);
        _testDC(i, .83f);
        _testDC(i, .17f);
    }
}

void testPoly()
{
    temp();
    test0();
    test1();
    testDC();
    testTerms();
}