
#include "OnsetDetector.h"

#include "TestGenerators.h"
#include <functional>
#include <memory>

#include "asserts.h"



//using generator = std::function<float(void)>;


/**
 * find the first onset and returns it.
 * has various assertions an the trigger.
 * asserts if not exactly one trigger.
 */
int findFirstOnset(TestGenerators::Generator g, int size, int numTriggersExpected)
{
    bool isActive = false;
    int triggerCount = 0;
    int firstOnset = -1;
    int triggerDuration = 0;

    OnsetDetector o;
    int index = 0;
    while (size--) {
        ++index;
        const float x = float(g());
        const bool detected = o.step(x);
        //if (detected) printf("DETECTED\n");

        const bool newTrigger = detected && !isActive;
        isActive = detected;

        if (newTrigger) {
            ++triggerCount;
            assert(triggerCount == 1);
        }
      

        if (newTrigger) {
            triggerDuration = -1;
        }

        if (newTrigger && firstOnset < 0) {
            firstOnset = index;
        }     

        if (isActive) {
            triggerDuration++;
        }
    }
    assert(!isActive);      // we want to see the detector go low
    assertEQ(triggerCount, numTriggersExpected);

    if (numTriggersExpected) {
        assertEQ(triggerDuration, 44);
    }
    return firstOnset;
}

static void test0()
{
    OnsetDetector o;
    o.step(0);
}

static void testStepGen()
{
    // want a sin that is an even multiple of
    
    TestGenerators::Generator g = TestGenerators::makeStepGenerator(1500);
    const int totalLen = 1500 + TestGenerators::stepDur + TestGenerators::stepTail;
    for (int i = 0; i < totalLen; ++i) {
        float expectedOut = i < 1500 ? 0.f : 1.f;
        expectedOut = (i >= 1500 + TestGenerators::stepDur) ? 0 : expectedOut;

        assertEQ(g(), expectedOut);
    }
}

static void testOnsetSin()
{
    printf("\n\n*** test onset sin\n");

    // 512/10 is even, works
    // TODO: two tests
    double period = 512 / 10.3;
    double freq = 1.0 / period;
    freq *= AudioMath::_2Pi;

   // double freq = .02;
    int x = findFirstOnset(
        TestGenerators::makeSinGenerator(freq),
        512 * 3,
        0);
    assertLT(x, 0);
}

static void testOnsetDetectPulse()
{
    printf("\n**** pulse\n");
    // let the test run long enough to see the pulse go low.
    int x = findFirstOnset(TestGenerators::makeStepGenerator(512 * 5 + 256), 512 * 9, 1);
    const int actualOnset = int(OnsetDetector::frameSize * 5.5);
    const int minOnset = OnsetDetector::preroll + OnsetDetector::frameSize * 5;
    const int maxOnset = OnsetDetector::preroll + OnsetDetector::frameSize * 6;
    assertGE(x, minOnset);
    assertLE(x, maxOnset);
}


static void testOnsetDetectStep()
{
    printf("\n**** step\n");
    // let the test run long enough to see the pulse go low.

    // try to find a freq so that the phase doesn't jump at the step
    int firstSectionLength = 512 * 5 + 256;


    // compute a freq close to what we want that will be even
    double desiredFreq = .005;      // normalized to Fs
    double desiredPeriod = 1 / desiredFreq;
    double desiredPeriodsInSection = firstSectionLength / desiredPeriod;

    double periodsInSection = std::round(desiredPeriodsInSection);
    double period = firstSectionLength / periodsInSection;
    double freq = 1 / period;

    freq *= AudioMath::_2Pi;

    //double evenPeriod = std::round(desiredPeriod);
    //double freq = 1 / evenPeriod;

    
    int x = findFirstOnset(
        TestGenerators::makeSteppedSinGenerator(512 * 5 + 256, freq, 8),
        512 * 9,
        1);
    const int actualOnset = int(OnsetDetector::frameSize * 5.5);
    const int minOnset = OnsetDetector::preroll + OnsetDetector::frameSize * 5;
    const int maxOnset = OnsetDetector::preroll + OnsetDetector::frameSize * 6;
    assertGE(x, minOnset);
    assertLE(x, maxOnset);
}

void testOnset2()
{
#if 0
    test0();
    testStepGen();
    printf("REMOVED FAILING SIN TEST TEMPORARILY");
    testOnsetSin();
    //testOnsetDetectPulse();
   // testOnsetDetectStep();
#endif
}