
#include "asserts.h"
#include "LadderFilter.h"


static void testLadderZero()
{
    LadderFilter<float> f;
    f.setFc(.01f);
    for (int i = 0; i < 20; ++i) {
        f.run(0);
        float x = f.getOutput();
        assertEQ(x, 0);
    }
}

void testLadder()
{
    testLadderZero();
}