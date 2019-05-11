
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


void testLadder()
{
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
   
}