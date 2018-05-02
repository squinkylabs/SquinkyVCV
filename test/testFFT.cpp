
#include "asserts.h"
#include <memory>
#include <set>

#include "AudioMath.h"
#include "FFTData.h"
#include "FFT.h"



static void testAccessors()
{
    FFTDataReal d0(16);

    d0.set(0, 4);
    assertEQ(d0.get(0), 4);

    FFTDataCpx dc(16);

    cpx x(3, 4);
    dc.set(5, x);
    assertEQ(dc.get(5), x);
}

static void testFFTErrors()
{
    FFTDataReal real(16);
    FFTDataCpx cpx(15);
    const bool b = FFT::forward(&cpx, real);
    assert(!b);          // should error if size mismatch
}

static void testForwardFFT_DC()
{
    FFTDataReal real(16);
    FFTDataCpx complex(16);

    // set real for DC
    for (int i = 0; i < 16; ++i) {
        real.set(i, 1.0);
    }
    const bool b = FFT::forward(&complex, real);
    assert(b);

    for (int i = 0; i < 16; ++i) {
        cpx v = complex.get(i);
        float mag = std::abs(v);
        float expect = (i == 0) ? 1.f : 0.f;
        assertEQ(mag, expect);
    }
}

static void test3()
{
    FFTDataReal real(16);
    FFTDataCpx complex(16);

    // set real for fundamental sin.
    // make peak 2.0 so fft will come out to one
    for (int i = 0; i < 16; ++i) {
        auto x = 2.0 *sin(AudioMath::Pi * 2.0 * i / 16.0);
        real.set(i, float(x));
    }
    const bool b = FFT::forward(&complex, real);
    assert(b);

    for (int i = 0; i < 16; ++i) {
        cpx v = complex.get(i);
        float mag = std::abs(v);
      
        float expect = (i == 1) ? 1.f : 0.f;
        assertClose(mag, expect, .0001);
    }
}


static void testRoundTrip()
{
    FFTDataReal realIn(16);
    FFTDataReal realOut(16);
    FFTDataCpx complex(16);

    // set real for DC
    for (int i = 0; i < 16; ++i) {
        realIn.set(i, 1.0);
    }

    bool b = FFT::forward(&complex, realIn);
    assert(b);
    b = FFT::inverse(&realOut, complex);

    for (int i = 0; i < 16; ++i) {
      
        float expect = 1.f; // scaled DC (TODO: fix scaling) 
        assertEQ(realOut.get(i) , expect);
    }
}


static void testNoiseFormula()
{
    const int bins = 1024 * 64;
    std::unique_ptr<FFTDataCpx> data(new FFTDataCpx(bins));
    assertEQ(data->size(), bins);

    FFT::makeNoiseSpectrum(data.get(), ColoredNoiseSpec());

    std::set<float> phases;

    for (int i = 0; i < bins; ++i) {
        const cpx x = data->get(i);
        float mag = std::abs(x);
        float phase = std::arg(x);

        const float expectedMag = (i == 0) ? 0.f : 1.f;
        assertClose(mag, expectedMag, .0001);
        //assert(phases.find(phase) == phases.end());
        //printf("adding phase %f\n", phase);
        phases.insert(phase);
    }
    printf("end of test, max=%d have%zd\n", bins, phases.size());
}

static float getPeak(const FFTDataReal& data)
{
    float peak = 0;
    for (int i = 0; i < data.size(); ++i) {
        peak = std::max(peak, std::abs(data.get(i)));
    }
    return peak;
}


static void testNoiseRTSub(int bins)
{
    std::unique_ptr<FFTDataCpx> dataCpx(new FFTDataCpx(bins));
    std::unique_ptr<FFTDataReal> dataReal(new FFTDataReal(bins));
    assertEQ(dataCpx->size(), bins);
    FFT::makeNoiseSpectrum(dataCpx.get(), ColoredNoiseSpec());

    FFT::inverse(dataReal.get(), *dataCpx);
    FFT::normalize(dataReal.get());

    const float peak = getPeak(*dataReal);
    assertClose( peak, 1.0f , .001);
}

static void testNoiseRT()
{
    testNoiseRTSub(4);
    testNoiseRTSub(8);
    testNoiseRTSub(16);
    testNoiseRTSub(1024);
    testNoiseRTSub(1024 * 64);
}


static void testPinkNoise()
{
    const int bins = 1024*4;
    std::unique_ptr<FFTDataCpx> data(new FFTDataCpx(bins));
    assertEQ(data->size(), bins);

    ColoredNoiseSpec spec;
    spec.highFreqCorner = 22100;        // makes no difference for - slope;
    spec.slope = -3;
    spec.sampleRate = 44100;

    FFT::makeNoiseSpectrum(data.get(), spec);


    // pick a starting bin above our 40 hz low freq corner
    const int baseBin = 16;
    float freqBase = 44100 * baseBin / (float) bins;
    assertGT (freqBase, 80);

    // mid-band, quadruple freq should reduce amp by 6db
    float mag16 = std::abs(data->get(baseBin));
    float mag64 = std::abs(data->get(4 * baseBin));

    // TODO: compare in db
    assertClose(mag16, 2 * mag64, .01);
}

static void testBlueNoise()
{
    const int bins = 1024 * 4;
    std::unique_ptr<FFTDataCpx> data(new FFTDataCpx(bins));
    assertEQ(data->size(), bins);

    ColoredNoiseSpec spec;
    spec.highFreqCorner = 8000;       
    spec.slope = 3;
    spec.sampleRate = 44100;

    FFT::makeNoiseSpectrum(data.get(), spec);


    assert(false);      // this is for pink - port it
    float freq16 = 44100 * 16 / (float) bins;
    assertGT(freq16, 20);

    // mid-band, quadruple freq should reduce amp by 6db
    float mag16 = std::abs(data->get(16));
    float mag64 = std::abs(data->get(64));

    assertClose(mag16, 2 * mag64, .001);
}

void testFFT()
{
    testAccessors();
    testFFTErrors();
    testForwardFFT_DC();
    test3();
    testRoundTrip();
    testNoiseFormula();
    testNoiseRT();
    testPinkNoise();
    testBlueNoise();
}