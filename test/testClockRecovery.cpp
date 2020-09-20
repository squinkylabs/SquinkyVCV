
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

static void generateNPeriods(ClockRecovery& c, int period, int times)
{
    assertGT(times, 0);

    const int firstHalfPeriod = period / 2;
    const int secondHalfPeriod = period - firstHalfPeriod;

    for (int i = 0; i < times; ++i) {
        for (int t = 0; t < period; ++t) {
            c.step(t < firstHalfPeriod ? 5.f : -5.f);
        }
    }
}

static void testClockRecoveryTwoPeriods()
{
    ClockRecovery c;
    c.step(-5);

    generateNPeriods(c, 10, 2);
    c.step(5.f);
    assertClose(c._getEstimatedFrequency(), .1f, .0001);
}

void testClockRecovery() 
{
    testClockRecoveryInit();
    testClockRecoveryOnePeriod();
    testClockRecoveryTwoPeriods();
}
