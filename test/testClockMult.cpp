
#include <assert.h>
#include "asserts.h"

#include "ClockMult.h"

static void test0()
{
    ClockMult cm;
    cm.setDivisor(1);
    const int period = 4;

    cm.refClock();  // give it an external clock

    // train with 4 ref clocks()
    for (int i = 0; i < period; ++i) {
        const int ck = cm.sampleClock();
        assertEQ(cm.getReset(), false);
        assert(ck == 1);
    }

    cm.refClock();      // now it knows the period is 4

    // verify nothing happens before period expires
    for (int i = 0; i < period-1; ++i) {
        const int ck = cm.sampleClock();
        assertEQ(cm.getReset(), false);
        assert(ck == 1);
    }

    
}

void testClockMult()
{
    test0();
}