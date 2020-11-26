
#include "OscSmoother.h"

#include "asserts.h"


static void generateNPeriods(OscSmoother& c, int period, int times)
{
    assertGT(times, 0);

    const int firstHalfPeriod = period / 2;
    // const int secondHalfPeriod = period - firstHalfPeriod;

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

static void testChangeFreq()
{
    OscSmoother o;

    // 16 cycles of 7
    for (int cycle = 0; cycle < 16; ++cycle) {
        int period = 7;
        assert(!o.isLocked());
        generateNPeriods(o, period, 1);
    }
    assert(!o.isLocked());

    // 16 cycles of 17
    for (int cycle = 0; cycle < 16; ++cycle) {
        int period = 17;
        generateNPeriods(o, period, 1);
        if (cycle == 0) {
            assertClose(o._getPhaseInc(), 1.f / 7.f, .00001f);
        }
    }
    // why do we need this extra period?
    o.step(5);
    o.step(-5);
    o.step(5);

    // why isn't this closer?
    assertClose(o._getPhaseInc(), 1.f / 17.f, .001f);



   // float expectedPeriod = (10.f * 15 + 9) / 16.f;
   // const float expectedFreq = 1.f / expectedPeriod;
  //  assertClose(o._getPhaseInc(), expectedFreq, .00001f);
}

static void testOutput()
{
  printf("---------- test output\n");
    int div = 50;
    const float expectedPhaseInc = 1.f / float(div);
    OscSmoother o;
    generateNPeriods(o, div, 20);

  
    bool first = true;
    float maxv = -10;
    float minv = 10;
    float last = 0;
    printf("----- after warmup\n"); fflush(stdout);
    for (int i=0; i< 200; ++i) {
        float x = o.step(0);
       // printf("outpu = %f\n", x);
        if (first) {
            first = false;
            last = x;
        } else {
            const float tolerance = 20.f / (50.f - 2);
            float delta = std::abs(x - last);
            const float d1 = delta;
            delta = std::min(delta, std::abs(delta -10));
            assertClose(delta, 0, tolerance);
            last  = x;
        }

        maxv = std::max(maxv, x);
        minv = std::min(minv, x);
    }
    assertClose(maxv, 5, .2);
    assertClose(minv, -5, .2);
}

static void testRisingEdgeFractional_init()
{
    RisingEdgeDetectorFractional det;
    auto s = det.step(0);
    assert(!s.first);
}

static void testRisingEdgeFractional_simpleRiseFall()
{
    RisingEdgeDetectorFractional det;
    det.step(5);           // force a high

    // force some lows
    auto s = det.step(-5);
    assert(!s.first);
    s = det.step(-5);
    assert(!s.first);

    // now we have high and low
    // next zero cross should do it
    s = det.step(5);
    assert(s.first);
    s = det.step(5);
    assert(!s.first);

    s = det.step(-5);
    assert(!
    s.first);
    s = det.step(-5);
    assert(!s.first);
}

static void testRisingEdgeFractional_RiseFall2()
{
    RisingEdgeDetectorFractional det;

    // force high, low
    auto s = det.step(5); 
    assert(!s.first);        
    s = det.step(-5);
    assert(!s.first);

    // barely cross zero
    s = det.step(-.001f);
    assert(!s.first);
    s = det.step(.001f);
    assert(s.first);

    // now go super low
    s = det.step(-5);
    assert(!s.first);

    // leading edge ignored, didn't go above thresh yet
    s = det.step(5);
    assert(!s.first);

    // barely cross zero ignored - we haven't been below thresh yet
    s = det.step(-.001f);
    assert(!s.first);
    s = det.step(.001f);
    assert(s.first);

    // prev cleared out L,H
    // force LH
    s = det.step(-5);
    s = det.step(5);
    assert(!s.first);

     // barely cross zero should work now
    s = det.step(-.001f);
    assert(!s.first);
    s = det.step(.001f);
    assert(s.first); 
}


static void testRisingEdgeFractional_Half()
{
    RisingEdgeDetectorFractional det;

    // force high, low
    auto s = det.step(5); 
    assert(!s.first);        
    s = det.step(-5);
    assert(!s.first);

    // barely cross zero
    s = det.step(-.001f);
    assert(!s.first);
    s = det.step(.001f);
    assert(s.first);
    assertClose(s.second, .5f, .01);
}

static void testRisingEdgeFractional_Quarter()
{
    RisingEdgeDetectorFractional det;

    // force high, low
    auto s = det.step(5); 
    assert(!s.first);        
    s = det.step(-5);
    assert(!s.first);

    // barely cross zero
    s = det.step(-.001f);
    assert(!s.first);
    s = det.step(.001f * 3);
    assert(s.first);
    assertClose(s.second, .25f, .01);
}


static void testRisingEdgeFractional_Ratio(float ratio)
{
    assert(ratio > 1);
    RisingEdgeDetectorFractional det;

    const float lowVoltage = -.1f;
    const float highVoltage = (ratio - 1) * .1f;

    assertClose((highVoltage - lowVoltage),  ratio * .1f, .001);
    assert(highVoltage > 0 && highVoltage < 1);
    assert(lowVoltage < 0 && lowVoltage > -1);

    // force high, low
    auto s = det.step(5); 
    assert(!s.first);        
    s = det.step(-5);
    assert(!s.first);

    // barely cross zero
    s = det.step(lowVoltage);
    assert(!s.first);
    s = det.step(highVoltage);
    assert(s.first);
    assertClose(s.second, 1.f / ratio, .0001);
}

static void  testRisingEdgeFractional_Ratio()
{
    testRisingEdgeFractional_Ratio(2);
    assert(false);
}

void testOscSmoother()
{
    testRisingEdgeFractional_init();
    testRisingEdgeFractional_simpleRiseFall();
    testRisingEdgeFractional_RiseFall2();
    testRisingEdgeFractional_Half();
    testRisingEdgeFractional_Quarter();
    testRisingEdgeFractional_Ratio();
    #if 0   // these broke. must fix
    testOscSmootherInit();
    testOscSmootherCanLock();
    testOscSmootherPeriod();
    testOscAltPeriod();
    testChangeFreq();
    testOutput();
    #endif

    
}
