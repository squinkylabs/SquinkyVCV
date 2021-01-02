
#include "ChaosKitty.h"
#include "TestComposite.h"
#include "asserts.h"

using Comp = ChaosKitty<TestComposite>;

void assertG(float g) {
    assert(g > 3.5);
    assert(g < 4);
}


static void testLabels()
{
    
    auto l = Comp::typeLabels();
    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    auto type = icomp->getParam(Comp::TYPE_PARAM);
    assertEQ(type.min, 0);
    assertEQ(type.max, l.size()-1);
}

static void testDC()
{
    ChaosGen1 chaos;
    chaos.onSampleRateChange(44100, 1.f / 44100);
    float sum = 0;
    for (int i = 0; i < 10000; ++i) {
        float x = chaos.step();
        assertLT(x, 500);
        assertGT(x, -500);
        sum += x;
    }
    assert(sum < 10);
    assert(sum > -10);
}

static void testReso()
{
    ResonantNoise chaos;
    chaos.onSampleRateChange(44100, 1.f / 44100);
  //  void set(float freqHz, float brightness, float resonance) {
    
    // these with the original negative feedback. 100k iterations
    //chaos.set(100, .2f, .9f);       // .001
    //chaos.set(100, .2f, 0.f);       // .003
    //chaos.set(100, .2f, .99f);      // .0001


    // these with positive feedback 100k
   // chaos.set(100, .2f, 0.f);  // .2
   // chaos.set(100, .2f, .9f);   // .05
    chaos.set(100, .2f, .99f);  // .005

    const int iterations = 1000000;

    float sum = 0;
    for (int i = 0; i < iterations; ++i) {
        float x = chaos.step();
        assertLT(x, 500);
        assertGT(x, -500);
        sum += x;
    }
    float dc = std::abs(sum / iterations);
    assertLT(dc, .1);       // there is some DC, not quite sure where it comes from
   // assert(sum < 10);
   // assert(sum > -10);
}

void testChaos()
{
    testLabels();
    testDC();
    testReso();
}