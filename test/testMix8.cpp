
#include "asserts.h"
#include "Mix8.h"
#include "TestComposite.h"

using Mixer = Mix8<TestComposite>;
using MixerPtr = std::shared_ptr<Mixer>;

static MixerPtr getMixer()
{
    MixerPtr ret = std::make_shared<Mixer>();
    ret->init();
    auto icomp = ret->getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        ret->params[i].value = param.def;
    }
    return ret;
}

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

static void testMaster(bool side)
{
    MixerPtr m = getMixer();

    m->inputs[Mixer::AUDIO0_INPUT].value = 10;
    m->params[Mixer::PAN0_PARAM].value = side ? -1.f : 1.f;     // full left

    m->step();
    float outL = m->outputs[Mixer::LEFT_OUTPUT].value;
    float outR = m->outputs[Mixer::RIGHT_OUTPUT].value;
    float expectedOutL = side ? float(10 * .8 * .8) : 0;
    float expectedOutR = side ? 0 : float(10 * .8 * .8);
    assertClose(outL, expectedOutL, .001);
    assertClose(outR, expectedOutR, .001);
}


static void testMaster()
{
    testMaster(false);
    testMaster(true);
}
void testMix8()
{
    testChannel();
    testMaster();
}