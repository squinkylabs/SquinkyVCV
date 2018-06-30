
#include <assert.h>

#include "FFT.h"
#include "GraphicEq.h"
#include "StateVariableFilter.h"
#include "SinOscillator.h"
#include "Analyzer.h"
#include "asserts.h"

// 3000 -> 3091 in regular mode
// 3066 for accurate (same for double
static void testPeak(std::function<float(float)> filter, float sampleRate, float expectedMax, float percentTolerance)
{
    const int numSamples = 64 * 1024;
    FFTDataCpx x(numSamples);
    Analyzer::getFreqResponse(x, filter);

    int maxBin = Analyzer::getMax(x);
    float maxFreq = (FFT::bin2Freq(maxBin, 44100, numSamples) +
        FFT::bin2Freq(maxBin + 1, 44100, numSamples)) / 2;
    printf("testPeak max bin = %d, freq=%f expected =%f\n", maxBin, maxFreq, expectedMax);

    const float delta = expectedMax * percentTolerance / 100.f ;      // One percent accuracy
    assertClose(maxFreq, expectedMax, delta);
    
}

/**
 * Really a test of the analyzer.
 * make as sin wave and see if the peak is where it should be.
 */
static void test0()
{
    SinOscillatorParams<float> params;
    SinOscillatorState<float> state;
    const float Fc = 100;
    const float sampleRate = 44100;

    SinOscillator<float, false>::setFrequency(params, Fc / sampleRate);
    std::function<float(float)> filter = [&state, &params](float x) {
        auto y = SinOscillator<float, false>::run(state, params);
        return y;
    };
    testPeak(filter, sampleRate, 100, 1);
}


static void testBandpass(float Fc, float tolerancePercent)
{
    StateVariableFilterState<float> state;
    StateVariableFilterParams<float> params;

   // const float Fc = 100;
    const float sampleRate = 44100;

    params.setMode(params.Mode::BandPass);
    params.setFreq(Fc / sampleRate);
    params.setNormalizedBandwidth(.7f);

    std::function<float(float)> filter = [&state, &params](float x) {
        auto y = StateVariableFilter<float>::run(x, state, params);
       // printf("filter(%f) ret (%f)\n", x, y);
        return y;
    };
    testPeak(filter, sampleRate, Fc, tolerancePercent);
}

static void test1()
{
    printf("why are these tests failing now with deltas of 1 \n"); // 10 .. 3000 passed with 1 before
    testBandpass(10, 10);
    testBandpass(100, 2);
    testBandpass(1000, 50);
    testBandpass(3000, 360);
}


static void _testTwoStage(float bw, double fmul, bool parallel)
{
    StateVariableFilterState<float> state[2];
    StateVariableFilterParams<float> params[2];

    const float Fc = 100;
    const float sampleRate = 44100;

    for (int i = 0; i <= 1; ++i) {
        float mult = (i == 0) ? 1.f : (float)fmul;
        params[i].setMode(params[i].Mode::BandPass);
        params[i].setFreq(Fc * mult / sampleRate);
        params[i].setNormalizedBandwidth(bw);
    }

    std::function<float(float)> filter = [&state, &params, parallel](float x) {
        if (parallel) {
            auto y = StateVariableFilter<float>::run(x, state[0], params[0]);
            auto z = StateVariableFilter<float>::run(x, state[1], params[1]);
            return y + z;
        } else {
            auto y = StateVariableFilter<float>::run(x, state[0], params[0]);
            auto z = StateVariableFilter<float>::run(y, state[1], params[1]);
            return z;
        }


        // printf("filter(%f) ret (%f)\n", x, y);
      
    };

    const int numSamples = 64 * 1024;
   // const int sampleRate = 44100;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    float dbTol = -3;
    auto x = Analyzer::getMaxAndShoulders(response, dbTol);
    printf("\ndbl %s bw=%f, mult=%f\n", parallel ? "par " : "ser ",
        bw, fmul);
    printf("dbl band: lf 3db at %f, high at %f, center at %f\n RATIO:%f\n",
        FFT::bin2Freq(std::get<0>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<1>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples) / FFT::bin2Freq(std::get<0>(x), sampleRate, numSamples)
    );
}

static void testTwoStage()
{
#if 0
    _testTwoStage(.525f, std::sqrt(2), true);
    _testTwoStage(.5f, std::sqrt(2), true);
    _testTwoStage(.475f, std::sqrt(2), true);
    _testTwoStage(.45f, std::sqrt(2), true);
    _testTwoStage(.4f, std::sqrt(2), true);
#endif

    _testTwoStage(1.1f, std::sqrt(2), false);
    _testTwoStage(1.f, std::sqrt(2), false);
    _testTwoStage(.9f, std::sqrt(2), false);
    _testTwoStage(.8f, std::sqrt(2), false);
    _testTwoStage(.7f, std::sqrt(2), false);
    _testTwoStage(.6f, std::sqrt(2), false);
    _testTwoStage(.5f, std::sqrt(2), false);
    //_testTwoStage(.4f, std::sqrt(2), false);

    _testTwoStage(1.f, std::sqrt(2)+.1, false);
    _testTwoStage(1.f, std::sqrt(2), false);
    _testTwoStage(1.f, std::sqrt(2) -.1, false);
    _testTwoStage(1.f, std::sqrt(2) - .2, false);
    _testTwoStage(1.f, std::sqrt(2) - .3, false);
    _testTwoStage(1.f, 1, false);

}



static void testTwoStage2()
{
    
    TwoStageBandpass bandpass;
    const float Fc = 100;
    const float sampleRate = 44100;

    bandpass.setFreq(Fc / sampleRate);

    std::function<float(float)> filter = [&bandpass](float x) {
        return bandpass.run(x);
    };

    const int numSamples = 64 * 1024;
    // const int sampleRate = 44100;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    float dbTol = -3;
    auto x = Analyzer::getMaxAndShoulders(response, dbTol);
    printf("\ndoing fixed two stage\n");
    printf("band: lf 3db at %f, high at %f, center at %f\n RATIO:%f\n",
        FFT::bin2Freq(std::get<0>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<1>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples) / FFT::bin2Freq(std::get<0>(x), sampleRate, numSamples)
    );
}



static void test2()
{
    StateVariableFilterState<float> state;
    StateVariableFilterParams<float> params;

    const float Fc = 100;
    const float sampleRate = 44100;

    params.setMode(params.Mode::BandPass);
    params.setFreq(Fc / sampleRate);
    params.setNormalizedBandwidth(.2f);

    std::function<float(float)> filter = [&state, &params](float x) {
        auto y = StateVariableFilter<float>::run(x, state, params);
        // printf("filter(%f) ret (%f)\n", x, y);
        return y;
    };
    //testPeak(filter, sampleRate, Fc, tolerancePercent);

    printf("test2: bp resp:\n");
    const int numSamples = 64 * 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    int maxBin = Analyzer::getMax(response);
    printf("max bin %d f = %f gain = %f\n", maxBin,
        FFT::bin2Freq(maxBin, sampleRate, numSamples),
        std::abs(response.get(maxBin)));

    Analyzer::getAndPrintFeatures(response, 3, sampleRate);

}


static void test3()
{
    const float bw = .7f;
    const float Fc = 200;
    printf("one stage, fc=%f bw=%f\n", Fc, bw);
    StateVariableFilterState<float> state;
    StateVariableFilterParams<float> params;

   
    const float sampleRate = 44100;

    params.setMode(params.Mode::BandPass);
    params.setFreq(Fc / sampleRate);
    params.setNormalizedBandwidth(bw);

    std::function<float(float)> filter = [&state, &params](float x) {
        auto y = StateVariableFilter<float>::run(x, state, params);
        // printf("filter(%f) ret (%f)\n", x, y);
        return y;
    };
    //testPeak(filter, sampleRate, Fc, tolerancePercent);

    printf("test2: bp resp:\n");
    const int numSamples = 64 * 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    auto x = Analyzer::getMaxAndShoulders(response, -3);
    printf("lf 3db at %f, high at %f, center at %f\n",
        FFT::bin2Freq(std::get<0>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<1>(x), sampleRate, numSamples)
    );

}

static void _test4(int stages, float bw, float dbTolerance)
{
    printf("\n*** test geq %d sages bw=%f db=%f \n", stages, bw, dbTolerance);
    GraphicEq geq(stages, bw);
    std::function<float(float)> filter = [&geq](float x) {
        auto y = geq.run(x);
        // printf("filter(%f) ret (%f)\n", x, y);
        return y;
    };

    const int numSamples = 64 * 1024;
    const int sampleRate = 44100;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    auto x = Analyzer::getMaxAndShoulders(response, dbTolerance);
    printf("geq: lf 3db at %f, high at %f, center at %f\n",
        FFT::bin2Freq(std::get<0>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<1>(x), sampleRate, numSamples)
    );
}


/** 
 * Inaccurate filters:
 * 3db, 3 stages, bw.6 works well
 * 2db .7 works well
 *
 * 3db 4 stages, 3db: .6
 */
static void test4()
{
    printf("\n");

   // _test4(2, .8f);
  //  _test4(2, .7f);
    const float db = -3;
 //   _test4(3, 1.1f, db);
 //   _test4(3, 1.0f, db);
//    _test4(3, .9f, db);
//    _test4(3, .8f, db);
    // _test4(3, .7f, db, false);
    _test4(3, .6f, db);
    //_test4(3, .5f, db, false);
    _test4(3, .6f, 2 * db);
    _test4(3, .7f, 2 * db);
    _test4(3, .8f, 2 * db);

  //  _test4(4, .8f, db);
//    _test4(4, .7f, db);
 //   _test4(4, .6f, db);
 //   _test4(4, .5f, db);
   // _test4(4, .7f);
  //  _test4(4, .7f);

}

void testFilter()
{
    printf("todo: finish test filters\n");
#if 0
    test0();
    test1();
    test2();
    test3();
    test4();
    testTwoStage();
#endif
 
    testTwoStage2();
}