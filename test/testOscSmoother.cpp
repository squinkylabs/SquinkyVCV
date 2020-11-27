
#include "OscSmoother.h"

#include "asserts.h"

template <class T>
static void generateNPeriods(T& c, int period, int times)
{
    assertGT(times, 0);

    c._primeForTest();          // make sure edge detector is setup up to interpret 5V as an edge

    const int firstHalfPeriod = period / 2;
    // const int secondHalfPeriod = period - firstHalfPeriod;

    for (int i = 0; i < times; ++i) {
        for (int t = 0; t < period; ++t) {
            const float input = t < firstHalfPeriod ? 5.f : -5.f;
#ifdef _OSL
            printf("generate sending input volt: %f\n", input);
#endif
            c.step(input);
        }
    }
}

template <class T>
static void testOscSmootherInit()
{
    T o;
    assertEQ(o.isLocked(), false);
}

template <class T>
static void testOscSmootherCanLock()
{
    T o;
    generateNPeriods(o, 6, 20);
    assertEQ(o.isLocked(), true);
}

template <class T>
static void testOscSmootherPeriod(int div)
{
    const float expectedPhaseInc = 1.f / float(div);
    T o;
    generateNPeriods(o, div, 20);
    assertEQ(o.isLocked(), true);
    assertClose(o._getPhaseInc(), expectedPhaseInc, .00001f);
}

template <class T>
static void testOscSmootherPeriod()
{
    testOscSmootherPeriod<T>(6);
    testOscSmootherPeriod<T>(10);
    testOscSmootherPeriod<T>(101);
}

template <class T>
static void testOscAltPeriod()
{
    T o;
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

template <class T>
static void testChangeFreq()
{
    T o;

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

template <class T>
static void testOutput()
{
    int div = 50;
    const float expectedPhaseInc = 1.f / float(div);
    T o;
    generateNPeriods(o, div, 20);

  
    bool first = true;
    float maxv = -10;
    float minv = 10;
    float last = 0;
    for (int i=0; i< 200; ++i) {
        float x = o.step(0);
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


static void testRisingEdgeFractional_Ratio(float ratio)
{
    assert(ratio > 0);
    RisingEdgeDetectorFractional det;

    const float scale = .01f;

    float lowVoltage = 0;
    float highVoltage = 0;
    float expectedSubsample = 0;
    if (ratio > 1) {
        lowVoltage = -scale;
        highVoltage = (ratio - 1) * scale;
        assertClose((highVoltage - lowVoltage), ratio * scale, .001);

        expectedSubsample = 1.f / ratio;
    }
    else {
        const float invRatio = 1 / ratio;
        highVoltage = scale;
        lowVoltage = (1 - invRatio ) * scale;
        expectedSubsample = 1 - ratio;
    }

    
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

   // printf("ratio = %f, hi=%f lo=%f\n", ratio, highVoltage, lowVoltage);
  //  printf("frac = %f\n", s.second);
    assertClose(s.second, expectedSubsample, .0001);
}

static void  testRisingEdgeFractional_Ratio()
{
    testRisingEdgeFractional_Ratio(2);
    testRisingEdgeFractional_Ratio(3);
    testRisingEdgeFractional_Ratio(4);
    testRisingEdgeFractional_Ratio(10);
    testRisingEdgeFractional_Ratio(100);

    testRisingEdgeFractional_Ratio(1.f / 2.f);
    testRisingEdgeFractional_Ratio(1.f / 3.f);
    testRisingEdgeFractional_Ratio(1.f / 2.f);
    testRisingEdgeFractional_Ratio(1.f / 4.f);
    testRisingEdgeFractional_Ratio(1.f / 10.f);
    testRisingEdgeFractional_Ratio(1.f / 100.f);
}

template <class T>
static void testOscSmootherT()
{
    testOscSmootherInit<T>();
    testOscSmootherCanLock<T>();
    testOscSmootherPeriod<T>();
 //   testOscAltPeriod<T>();
 //   testChangeFreq<T>();
  //  testOutput<T>();
}

void testOscSmoother()
{
#if 1
    testRisingEdgeFractional_init();
    testRisingEdgeFractional_simpleRiseFall();
    testRisingEdgeFractional_RiseFall2();
    testRisingEdgeFractional_Ratio();
#endif
    testOscSmootherT<OscSmoother>();
    testOscSmootherT<OscSmoother2>();    
}
