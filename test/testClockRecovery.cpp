
#include "ClockRecovery.h"
#include "asserts.h"

// test initial conditions
static void testClockRecoveryInit()
{
    ClockRecovery c;
    assert(c.getState() == ClockRecovery::States::INIT);
    assertEQ(c._getResetCount(), 0);
    assertEQ(c._getResetCount(), 0);        // not acquired yet
    assertEQ(c._getEstimatedFrequency(), 0);

    bool b = c.step(0);
    assert(!b);
    assertEQ(c._getResetCount(), 0);
    assert(c.getState() == ClockRecovery::States::INIT);

   
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

    assert(c.getState() == ClockRecovery::States::LOCKING);
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
    assert(c.getState() == ClockRecovery::States::LOCKING);
    assertClose(c._getEstimatedFrequency(), .1f, .0001);
}

static void testClockRecoveryAlternatingPeriods()
{
    printf("\n\n---- start alt\n");
    ClockRecovery c;
    c.step(-5);
    for (int i = 0; i < 2000; ++i) {
        generateNPeriods(c, 10, 1);
        generateNPeriods(c, 11, 1);
    }
    // TODO:
    // assert(c.getState() == ClockRecovery::States::LOCKED);
    assertClose(c._getFrequency(), 1.f / 10.5f, .001);

}

void testClockRecovery() 
{
#if 0
    printf("skipping test clock recovery\n");
#else
    testClockRecoveryInit();
    testClockRecoveryOnePeriod();
    testClockRecoveryTwoPeriods();
    testClockRecoveryAlternatingPeriods();
#endif
}
