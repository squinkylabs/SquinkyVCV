#include "FFTCrossFader.h"
#include "ColoredNoise.h"
#include "asserts.h"

static void test0()
{
    FFTCrossFader f(4);
    NoiseMessage m(10);
    assertEQ(m.dataBuffer->get(0), 0);
    assertEQ(m.dataBuffer->get(9), 0);

    NoiseMessage* t = f.acceptData(&m);
    assertEQ(t, 0);
}


void testFFTCrossFader()
{
    assertEQ(FFTDataReal::_count, 0);
    test0();
    assertEQ(FFTDataReal::_count, 0);
}