
#include "OnsetDetector.h"

#include <functional>
#include <memory>

#include "asserts.h"



using generator = std::function<float(void)>;

int findFirstOnset(generator g, int size)
{
    OnsetDetector o;
    int index = 0;
    while (size--) {
        ++index;
        const float x = g();
        if (o.step(x)) {
            return index;
        }
    }
    return -1;
}

const int stepDur = 100;
const int stepTail = 1024;

generator makeStepGenerator(int stepPos)
{
    std::shared_ptr<int> counter = std::make_shared<int>(0);
    return [counter, stepPos]() {
        float ret = -1;
       
        if (*counter < stepPos) {
            ret = 0;
        } else if (*counter < stepPos + stepDur) {
            ret = 1;
        } else {
            ret = 0;
        }
        ++* counter;
       
        return ret;
    };
}

static void test0()
{
    OnsetDetector o;
    o.step(0);
}

static void testStepGen()
{
    generator g = makeStepGenerator(1500);
    const int totalLen = 1500 + stepDur + stepTail;
    for (int i = 0; i < totalLen; ++i) {
        float expectedOut = i < 1500 ? 0.f : 1.f;
        expectedOut = (i >= 1500 + stepDur) ? 0 : expectedOut;

        assertEQ(g(), expectedOut);
    }
}

static void testOnsetDetectPulse()
{
    int x = findFirstOnset(makeStepGenerator(512 * 5 + 256), 512 * 8);
    const int actualOnset = int(OnsetDetector::frameSize * 5.5);

    const int minOnset = OnsetDetector::preroll + OnsetDetector::frameSize * 5;
    const int maxOnset = OnsetDetector::preroll + OnsetDetector::frameSize * 6;
    assertGE(x, minOnset);
    assertLE(x, maxOnset);
}

void testOnset2()
{
    test0();
    testStepGen();
    testOnsetDetectPulse();
}