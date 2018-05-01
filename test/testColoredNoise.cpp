
#include "ColoredNoise.h"
#include "TestComposite.h"

using Noise = ColoredNoise<TestComposite>;

static void test0()
{
    {
        Noise cn;
        cn.init();
        // calling step should get client to request an FFT frame
        while (cn._msgCount() < 1) {
            cn.step();
        }
        printf("leaving test, will destroy cn\n");
    }

}
void testColoredNoise()
{
    test0();
}