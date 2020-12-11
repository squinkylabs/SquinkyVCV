
#include "ObjectCache.h"
#include "OscSmoother.h"

#include "asserts.h"

template <class T>
static void generateNPeriods(T& c, int period, int times)
{
    assertGT(times, 0);

    c._primeForTest(-.0001f);          // make sure edge detector is setup up to interpret 5V as an edge

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

class SimpleSine
{
public:
    SimpleSine(float inc, float init) : normPhase(init), normPhaseInc(inc) {}
    float step() {
        float ret = 5 * LookupTable<float>::lookup(*lookup, normPhase);
        normPhase += normPhaseInc;
        if (normPhase > 1) {
            normPhase -= 1;
        }
        return ret;
    }
    float normPhase = 0;
    float normPhaseInc = 0;
    std::shared_ptr<LookupTableParams<float>> lookup = ObjectCache<float>::getSinLookup();
};

static void testSimpleSine()
{
    SimpleSine osc(.25f, 0);
    assertEQ(osc.normPhase, 0);
    assertEQ(osc.normPhaseInc, .25f);

    float x = osc.step();
    assertClose(x, 0, .00001f);
    x = osc.step();
    assertClose(x, 5, .00001f);
    x = osc.step();
    assertClose(x, 0, .00001f);
    x = osc.step();
    assertClose(x, -5, .00001f);
    x = osc.step();
    assertClose(x, 0, .00001f);
    x = osc.step();
    assertClose(x, 5, .00001f);

    SimpleSine osc2(.25f, .125f);
    x = osc2.step();
    assertClose(x, 5 / std::sqrt(2.f), .00001f);
    x = osc2.step();
    assertClose(x, 5 / std::sqrt(2.f), .00001f);
    x = osc2.step();
    assertClose(x, -5 / std::sqrt(2.f), .00001f);
    x = osc2.step();
    assertClose(x, -5 / std::sqrt(2.f), .00001f);
}

template <class T>
static void generateFractionalPeriods(T& smoother, float period, int times)
{
    printf("----------- generateFractionalPeriods (%f)\n", period);
  //  smoother._primeForTest(-.0001f);

    // we want the first click to happen really close to the first sample
    smoother._primeForTest(-5);

    const float freq = 1.f / period;
    SimpleSine osc(freq, .00001f);
    const int totalSamples = 2 +  int(times * period);
    printf("times=%d total=%d\n", times, totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        float x = osc.step();
        smoother.step(x);
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
    printf("\n---- testOscSmootherPeriod %d\n", div);
    const float expectedPhaseInc = 1.f / float(div);
    T o;
    generateNPeriods(o, div, 20);
    assertEQ(o.isLocked(), true);
    assertClose(o._getPhaseInc(), expectedPhaseInc, .001f);
}

template <class T>
static void testOscSmootherPeriod()
{
    testOscSmootherPeriod<T>(6);
    testOscSmootherPeriod<T>(10);
    testOscSmootherPeriod<T>(101);
}

class TestParams
{
public:
    float period = 1;       // the period of the tests signal, in cycles
    int cycles = 1;         // how much test signal to generate
    bool expectLock = true;
    int periodOfSmoother = 16;
    float tolerance = .00001f;
};

template <class T>
static void testOscFractionalPeriod(const TestParams& params)
{
    printf("\n----------- testOscFractionalPeriod (%f)\n", params.period);
    printf("  test cycles = %d smoother period = %d\n", params.cycles, params.periodOfSmoother);
    const float expectedPhaseInc = 1.f / params.period;
    T o (params.periodOfSmoother);
    generateFractionalPeriods(o, params.period, params.cycles);
    assertEQ(o.isLocked(), params.expectLock);
    if (params.expectLock) {
        assertClose(o._getPhaseInc(), expectedPhaseInc, params.tolerance);
    }
}

template <class T>
static void testOscFractionalPeriod()
{

    TestParams p2;
    p2.cycles = 1;
    p2.periodOfSmoother = 1;

    p2.period = 4;
    testOscFractionalPeriod<T>(p2);

    p2.period = 4.5;
    testOscFractionalPeriod<T>(p2);

    p2.period = 4.6f;
    p2.tolerance = .001f;        // at high freq assumptions about linear approx aren't good.
                                // so increase the tolerance
    testOscFractionalPeriod<T>(p2);


    TestParams p;
    p.period = 4.5;
    p.cycles = 10;
    p.periodOfSmoother = 2;
    testOscFractionalPeriod<T>(p);


  //  TestParams p;
    p.period = 4.5;
    p.cycles = 20;
    p.periodOfSmoother = 16;
    testOscFractionalPeriod<T>(p);

    p.cycles = 10;
    p.expectLock = false;
    testOscFractionalPeriod<T>(p);

    p.cycles = 40;
    p.expectLock = true;
    testOscFractionalPeriod<T>(p);
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
    assertClose(o._getPhaseInc(), expectedFreq, .001f);
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


    assertClose(o._getPhaseInc(), 1.f / 17.f, .001f);
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

/**
 * like the ratio tests, but a simple
 * case of the cross happening right before current sample
 */
static void testRisingEdgeFractional_Close()
{
    RisingEdgeDetectorFractional det;
    det.step(5);
    det.step(-5);
    det.step(-5);
    auto x = det.step(.0001f);
    assert(x.first);
    assertLE(x.second, .0001f);
}

/**
 * like the ratio tests, but a simple
 * case of the cross happening right after prev sample
 */
static void testRisingEdgeFractional_Far()
{
    RisingEdgeDetectorFractional det;
    det.step(5);
    det.step(-5);
    det.step(-5);
    det.step(-.0001f);
    auto x = det.step(5);
    assert(x.first);
    assertGE(x.second, .999f);
}

static void testRisingEdgeFractional_HighFreq()
{
    RisingEdgeDetectorFractional det;
    det.step(5);
    det.step(-5);
    auto x = det.step(5);
    assert(x.first);
    assertClose(x.second, .5, .001);
    x = det.step(-5);
    assert(!x.first);

    x = det.step(5);
    assert(x.first);
    assertClose(x.second, .5, .001);
    x = det.step(-5);
    assert(!x.first);
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

       // expectedSubsample = 1.f / ratio;
       // did this backwards origianally
       expectedSubsample = 1 - (1.f / ratio);
    }
    else {
        const float invRatio = 1 / ratio;
        highVoltage = scale;
        lowVoltage = (1 - invRatio ) * scale;
        //expectedSubsample = 1 - ratio;
        expectedSubsample = ratio;
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
    testOscAltPeriod<T>();
    testChangeFreq<T>();
    testOutput<T>();
}

void testOscSmoother()
{
#if 1
    testSimpleSine();
    testRisingEdgeFractional_init();
    testRisingEdgeFractional_simpleRiseFall();
    testRisingEdgeFractional_RiseFall2();
    testRisingEdgeFractional_Close();
    testRisingEdgeFractional_Far();
    testRisingEdgeFractional_Ratio();
    testRisingEdgeFractional_HighFreq();


    testOscSmootherT<OscSmoother<double>>();
    testOscSmootherT<OscSmoother2<double>>();
#endif

    testOscFractionalPeriod<OscSmoother2<double>>();
}
