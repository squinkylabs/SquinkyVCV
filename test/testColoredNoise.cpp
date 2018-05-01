
#include "ColoredNoise.h"
#include "TestComposite.h"

using Noise = ColoredNoise<TestComposite>;

static void test0()
{
    printf("--- testColoredNoise 0\n");
    {
        Noise cn;
        cn.init();
        // calling step should get client to request an FFT frame
        while (cn._msgCount() < 1) {
            cn.step();
        }
        printf("leaving testColoredNoise 0, will destroy cn\n");
    }

}
void testColoredNoise()
{
    test0();
}