
#include "asserts.h"
#include "FFTData.h"


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

void testFFT()
{
    test0();
}