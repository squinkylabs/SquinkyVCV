
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



static void testOscSmootherPeriod(int div)
{
    const float expectedPhaseInc = 1.f / float(div);
    OscSmoother o;
    generateNPeriods(o, div, 20);
    assertEQ(o.isLocked(), true);
    assertClose(o._getPhaseInc(), expectedPhaseInc, .0001);
}

static void testOscSmootherPeriod()
{
    testOscSmootherPeriod(6);
    testOscSmootherPeriod(10);
}

void testOscSmoother()
{
    testOscSmootherInit();
    testOscSmootherCanLock();
    testOscSmootherPeriod();

}
