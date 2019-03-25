
#include "asserts.h"
#include "Mix8.h"
#include "TestComposite.h"

using Mixer = Mix8<TestComposite>;

static void testChannel(int channel)
{
    Mixer m;
    m.init();

    // zero all inputs, put all channel gains to 1
    for (int i = 0; i < 8; ++i) {
        m.inputs[Mixer::AUDIO0_INPUT + i].value = 0;
        m.params[Mixer::LEVEL0_INPUT + i].value = 1;          
    }

    m.inputs[Mixer::AUDIO0_INPUT + channel].value = 5.5f;
    m.params[Mixer::GAIN0_PARAM + channel].value = 2;

    m.step();

    for (int i = 0; i < 8; ++i) {
        float expected = (i == channel) ? 11.f : 0;
        assertClose(m.outputs[Mixer::CHANNEL0_OUTPUT + i].value, expected, .00001f);
    }

}

static void testChannel()
{
    for (int i = 0; i < 8; ++i) {
        testChannel(i);
    }
}

void testMix8()
{
    testChannel();
}