
#include "asserts.h"
#include "LadderFilter.h"


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

void testLadder()
{
    testLadderZero();
    testLadderNotZero();
}