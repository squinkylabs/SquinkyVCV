
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
    }
}

static void test1()
{
    printf("test1\n");
    Noise cn;
    cn.init();
    // calling step should get client to request an FFT frame
    int num = 0;
    bool started = false;
    for (bool done = false; !done; ) {
        cn.step();
        const float output = cn.outputs[Noise::AUDIO_OUTPUT].value;
        if (output > .1) {;
            started = true;
        }
        assert(output < 10);
        assert(output > -10);
        if (started) {
            num++;
            if (num > 100000) {
                done = true;
            }
        }
    }

}
void testColoredNoise()
{
    test0();
    test1();
}