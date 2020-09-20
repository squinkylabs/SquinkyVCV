
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
    b = c.step(-5);
    assert(!b);

    b = c.step(5);
    assert(!b);
    b = c.step(5);
    assert(!b);
    b = c.step(5);
    assert(!b);

    b = c.step(-5);
    assert(!b);
    b = c.step(-5);
    assert(!b);
    b = c.step(-5);
    assert(!b);

    b = c.step(5);
    assert(b);

    assertClose(c._getEstimatedFrequency(), 1.f / 6.f, .001);
}

static void generateOnePeriod(ClockRecovery& c, int halfPeriod, int times)
{
    assertGT(times, 0);

    for (int i = 0; i < times; ++i) {
        for (int t = 0; t < halfPeriod * 2; ++t) {
            c.step(t < halfPeriod ? 5.f : -5.f);
        }
    }
}

static void testClockRecoveryTwoPeriods()
{
    ClockRecovery c;
    c.step(-5);

    generateOnePeriod(c, 5, 1);
    c.step(5.f);
    assertClose(c._getEstimatedFrequency(), .1f, .0001);
}

void testClockRecovery() 
{
    testClockRecoveryInit();
    testClockRecoveryOnePeriod();
    testClockRecoveryTwoPeriods();
}
