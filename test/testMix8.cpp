
#include "asserts.h"
#include "Mix8.h"
#include "TestComposite.h"

using Mixer = Mix8<TestComposite>;

static void testChannel(int channel, bool useParam)
{
    Mixer m;
    m.init();

    const float activeParamValue = useParam ? 2.f : 1.f;
    const float activeCVValue = useParam ? 5.f : 10.f;

    // zero all inputs, put all channel gains to 1
    for (int i = 0; i < 8; ++i) {
        m.inputs[Mixer::AUDIO0_INPUT + i].value = 0;
        m.params[Mixer::GAIN0_PARAM + i].value = 1;
    }

    m.inputs[Mixer::AUDIO0_INPUT + channel].value = 5.5f;
    m.params[Mixer::GAIN0_PARAM + channel].value = activeParamValue;
    m.inputs[Mixer::LEVEL0_INPUT + channel].value = activeCVValue;
    m.inputs[Mixer::LEVEL0_INPUT + channel].active = true;

    m.step();

//    assert(useParam);
    for (int i = 0; i < 8; ++i) {
        float expected = (i == channel) ? 5.5f : 0;
        assertClose(m.outputs[Mixer::CHANNEL0_OUTPUT + i].value, expected, .00001f);
    }

}

static void testChannel()
{
    for (int i = 0; i < 8; ++i) {
        testChannel(i, true);
        testChannel(i, false);
    }
}

void testMix8()
{
    testChannel();
}