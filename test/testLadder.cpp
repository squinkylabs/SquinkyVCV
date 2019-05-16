
#include "asserts.h"
#include "Filt.h"
#include "LadderFilter.h"
#include "TestComposite.h"


static void testLadderZero()
{
    LadderFilter<float> f;
    f.setNormalizedFc(.01f);
    for (int i = 0; i < 20; ++i) {
        f.run(0);
        float x = f.getOutput();
        assertEQ(x, 0);
    }
}

static void testLadderNotZero()
{
    LadderFilter<float> f;
    f.setNormalizedFc(.1f);
    for (int i = 0; i < 20; ++i) {
        f.run(1);
        float x = f.getOutput();
        assertGT(x, 0);
    }
}

template <typename T>
static void setupFilter(LadderFilter<T>& f)
{
    f.setType(LadderFilter<T>::Types::_3PHP);
    f.setNormalizedFc(1 / T(40));
    f.setBassMakeupGain(1);
    f.setVoicing(LadderFilter<T>::Voicing::Clean);
    f.setFeedback(0);

    f.setEdge(.5);
   
    f.setFeedback(0);
    f.setFreqSpread(0);
    f.setGain(1);
}

static void testLadderDCf(int repeats)
{
    LadderFilter<float> f;
    setupFilter(f);

    const float x = 3.78f;
   // const int repeats = 10;
    float y = 100;
    for (int i = 0; i < repeats; ++i) {
        f.run(x);
        y = f.getOutput();
      
    }
    y = f.getOutput();
    printf("testLadderDCf rep = %d out=%e\n", repeats, y);
    if (repeats >= 1000) assertClose(y, 0, 3.3e-5);
}

static void testLadderDCd(int repeats)
{
    LadderFilter<double> f;
    setupFilter(f);

    const double x = 3.78;
    double y = 100;
    for (int i = 0; i < repeats; ++i) {
        f.run(x);

    }
    y = f.getOutput();
    printf("testLadderDCd rep = %d out=%e\n", repeats, y);
   // .02 after 100+
    // 0 after 1000

    // better inits, 0 after 1000
    if (repeats >= 1000) assertClose(y, 0, 3e-6);
}

static void testLadderTypes()
{
    for (int i = 0; i < (int) LadderFilter<double>::Types::NUM_TYPES; ++i) {
        LadderFilter<double> f;
        f.setType(LadderFilter<double>::Types(i));
    }
}

// if not 4P LP, will all be zero
static void testLED0()
{
    LadderFilter<double> f;
    f.setType(LadderFilter<double>::Types::_1PHP);
    f.setSlope(2);
    for (int i = 0; i < 4; ++i) {
        assertEQ(f.getLEDValue(i), 0);
    }
}

static void testLED1()
{
    LadderFilter<double> f;
    f.setType(LadderFilter<double>::Types::_4PLP);
    f.setSlope(0);

    assertClose(f.getLEDValue(0), 1, .01);
    assertEQ(f.getLEDValue(1), 0);
    assertEQ(f.getLEDValue(2), 0);
    assertEQ(f.getLEDValue(3), 0); 
}

static void testLED2()
{
    LadderFilter<double> f;
    f.setType(LadderFilter<double>::Types::_4PLP);
    f.setSlope(1);

    assertClose(f.getLEDValue(1), 1, .01);
    assertEQ(f.getLEDValue(0), 0);
    assertEQ(f.getLEDValue(2), 0);
    assertEQ(f.getLEDValue(3), 0);
}

static void testLED3()
{
    LadderFilter<double> f;
    f.setType(LadderFilter<double>::Types::_4PLP);
    f.setSlope(2);

    assertClose(f.getLEDValue(2), 1, .01);
    assertEQ(f.getLEDValue(0), 0);
    assertEQ(f.getLEDValue(1), 0);
    assertEQ(f.getLEDValue(3), 0);
}

static void testLED4()
{
    LadderFilter<double> f;
    f.setType(LadderFilter<double>::Types::_4PLP);
    f.setSlope(3);

    assertClose(f.getLEDValue(3), 1, .01);
    assertEQ(f.getLEDValue(0), 0);
    assertEQ(f.getLEDValue(2), 0);
    assertEQ(f.getLEDValue(1), 0);
}

static void testLED5()
{
    LadderFilter<double> f;
    f.setType(LadderFilter<double>::Types::_4PLP);
    f.setSlope(2.5);

    assertClose(f.getLEDValue(3), .5, .01);
    assertEQ(f.getLEDValue(0), 0);
    assertClose(f.getLEDValue(2), .5, .01);
    assertEQ(f.getLEDValue(1), 0);
}

static void testFilt()
{
    Filt<TestComposite> f;

    assert(!f.getVoicingNames().empty());
    assert(!f.getTypeNames().empty());

    assertEQ(f.getVoicingNames().size(), (int)LadderFilter<float>::Voicing::NUM_VOICINGS);
    assertEQ(f.getTypeNames().size(), (int) LadderFilter<float>::Types::NUM_TYPES);

}

static void testFilt2()
{
    Filt<TestComposite> f;
    auto x = f.getTypeNames();
    for (auto t : x) {
        assert(!t.empty());
    }

    auto y = f.getVoicingNames();
    for (auto t : y) {
        assert(!t.empty());
    }
}

static bool _testFiltStability(double fNorm, double feedback)
{
    LadderFilter<double> f;
    f.setType(LadderFilter<double>::Types::_4PLP);
    f.setVoicing(LadderFilter<double>::Voicing::Clean);
    f.setGain(1);
    f.setEdge(.5);
    f.setFreqSpread(0);
    f.setBassMakeupGain(1);
    f.setNormalizedFc(fNorm);     // (.05) at .01 is stable at 3.99  .1 stable at 3.4
    f.setSlope(4);
    f.setFeedback(feedback);       // 5 unstable at 200, 4 unstable at 800 3.9 unstable at 1200
                                // 3.75 unstable at 6000
                                // 3.7 stable

   // AudioMath::RandomUniformFunc random = AudioMath::random();
#ifdef _TEXTEX
    const int  reps = 100000;
#else
    const int  reps = 10000;
#endif

    double a = 0, b = 0;
    for (int i = 0; i < reps; ++i) {
     //   const double noise = .1 * (random() - .5);
        const double noise = -.01;
        f.run(noise);
        double x = f.getOutput();
        if ((x < -1) || (x > 1)) {
         //   printf("over at i = %d\n", i);
            f.getOutput();
            f.run(noise);
            return false;
        }
        a = std::min(a, x);
        b = std::max(b, x);
        assert(x < 1);
        assert(x > -1);
       // printf("output = %f\n", x);
    }
  //  printf("LADDER extremes were %f, %f\n", a, b);
    return true;
}


// This is now baked into the ladder
static double getFeedForTest(double fNorm)
{
    double ret = 3.99;

    if (fNorm <= .002) {
        ret = 3.99;
    } else if (fNorm <= .008) {
        ret = 3.9;
    } else if (fNorm <= .032) {
        ret = 3.8;
    } else if (fNorm <= .064) {
        ret = 3.6;
    } else if (fNorm <= .128) {
        ret = 2.95;
    } else if (fNorm <= .25) {
        ret = 2.85;
    } else {
        ret = 2.30;
    }
    return ret;

}

static void testFiltStability()
{
#ifdef _TESTEX   // full test
    for (double f = .001; f < .5; f *= 2) {
     //   const double feed = getFeedForTest(f);
        const double feed = 4;
        bool stable = _testFiltStability(f, feed);
        printf("freq %f feed = %f stable=%d\n", f, feed, stable);
        assert(stable);

    }

    printf("\n");
    for (double f = .25; f < .7; f *= 1.1) {
        //const double feed = getFeedForTest(f);
        const double feed = 4;
        bool stable = _testFiltStability(f, feed);
        printf("freq %f feed = %f stable=%d\n", f, feed, stable);
        assert(stable);

    }
#endif

    assert(_testFiltStability(.01, 4));
    assert(_testFiltStability(.05, 4));
}


void testLadder()
{
#if 1
    testLadderZero();
    testLadderNotZero();
    testLadderDCf(1000);
    testLadderDCd(1000);
    testLadderTypes();
    testLED0();
    testLED1();
    testLED2();
    testLED3();
    testLED4();
    testLED5();
    testFilt();
    testFilt2();
#endif
    testFiltStability();
   
}