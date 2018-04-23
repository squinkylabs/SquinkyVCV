
#include "asserts.h"
#include "FFTData.h"
#include "FFT.h"


static void test0()
{
    FFTDataReal d0(16);

    d0.set(0, 4);
    assertEQ(d0.get(0), 4);

    FFTDataCpx dc(16);

    cpx x(3, 4);
    dc.set(5, x);
    assertEQ(dc.get(5), x);
}

static void test1()
{
    FFTDataReal real(16);
    FFTDataCpx cpx(15);
    const bool b = FFT::forward(&cpx, real);
    assert(!b);          // should error if size mismatch
}

static void test2()
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
       float magSq = v.imag() *v.imag() + v.real() * v.real();
       magSq = std::abs(v);
        float expect = (i == 0) ? 16.f : 0.f;
        assertEQ(magSq, expect);
        //real.set(i, 1.0);
    }

}
void testFFT()
{
    test0();
    test1();
    test2();
}