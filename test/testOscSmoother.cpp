
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
    printf("---------- test osc over %d\n", div);
    const float expectedPhaseInc = 1.f / float(div);
    OscSmoother o;
    generateNPeriods(o, div, 20);
    assertEQ(o.isLocked(), true);
    assertClose(o._getPhaseInc(), expectedPhaseInc, .00001f);
}

static void testOscSmootherPeriod()
{
    testOscSmootherPeriod(6);
    testOscSmootherPeriod(10);
    testOscSmootherPeriod(101);
}

static void testOscAltPeriod()
{
    printf("---------- test ALT\n");
    OscSmoother o;
    for (int cycle = 0; cycle < 16; ++cycle) {
        int period = (cycle == 0) ? 9 : 10;
        assert(!o.isLocked());
        generateNPeriods(o, period, 1);      // 1
    }
    assert(!o.isLocked());
    o.step(5);
    assert(o.isLocked());

    float expectedPeriod = (10.f * 15 + 9) / 16.f;
    const float expectedFreq = 1.f / expectedPeriod;
    assertClose(o._getPhaseInc(), expectedFreq, .00001f);

}


void testOscSmoother()
{
    testOscSmootherInit();
    testOscSmootherCanLock();
    testOscSmootherPeriod();
    testOscAltPeriod();
}
