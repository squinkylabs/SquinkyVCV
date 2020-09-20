
#include "ClockRecovery.h"
#include "asserts.h"

// test initial conditions
static void testClockRecoveryInit()
{
    ClockRecovery c;
    assertEQ(c._getResetCount(), 0);
    assertEQ(c._getResetCount(), 0);        // not acquired yet
    assertEQ(c._getEstimatedFrequency(), 0);

    bool b = c.step(0);
    assert(!b);
    assertEQ(c._getResetCount(), 0);
}

static void testClockRecoveryOnePeriod()
{
    ClockRecovery c;
    bool b;
    b = c.step(0);
    assert(!b);

    b = c.step(10);
    assert(!b);
    b = c.step(10);
    assert(!b);
    b = c.step(10);
    assert(!b);

    b = c.step(0);
    assert(!b);
    b = c.step(0);
    assert(!b);
    b = c.step(0);
    assert(!b);

    b = c.step(10);
    assert(b);


}

void testClockRecovery() 
{
    testClockRecoveryInit();
    testClockRecoveryOnePeriod();
}
