
#include "ColoredNoise.h"
#include "TestComposite.h"

using Noise = ColoredNoise<TestComposite>;

static void test0()
{
    {
        Noise cn;
        cn.init();
        for (int i = 0; i < 10; ++i) {
            cn.step();
        }
        printf("leaving test, will destroy cn\n");
    }

}
void testColoredNoise()
{
    test0();
}