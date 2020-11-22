
#include "OscSmoother.h"

#include "asserts.h"


static void generateNPeriods(OscSmoother& c, int period, int times)
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

static void testOscSmootherInit()
{
    OscSmoother o;
    assertEQ(o.isLocked(), false);
}

static void testOscSmootherCanLock()
{
    OscSmoother o;
    generateNPeriods(o, 6, 20);
    assertEQ(o.isLocked(), true);
}

void testOscSmoother()
{
    testOscSmootherInit();
    testOscSmootherCanLock();

}
