
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
    FFT::makeNoiseFormula(data.get(), 0, 0, 44100);

    std::set<float> phases;

    for (int i = 0; i < bins; ++i) {
        const cpx x = data->get(i);
        float mag = std::abs(x);
        float phase = std::arg(x);
        assertClose(mag, 1.0, .0001);
        //assert(phases.find(phase) == phases.end());
        //printf("adding phase %f\n", phase);
        phases.insert(phase);
    }
    printf("end of test, max=%d have%zd\n", bins, phases.size());

}

void testFFT()
{
    testAccessors();
    testFFTErrors();
    testForwardFFT_DC();
    test3();
    testRoundTrip();
    testNoiseFormula();

}