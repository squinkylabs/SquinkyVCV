
#include "asserts.h"
#include "Mix8.h"
#include "ObjectCache.h"
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

    for (int i = 0; i < 1000; ++i) {
        m.step();           // let mutes settle
    }

    for (int i = 0; i < 8; ++i) {
        float expected = (i == channel) ? 5.5f : 0;
        assertClose(m.outputs[Mixer::CHANNEL0_OUTPUT + i].value, expected, .01f);
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

    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }
    float outL = m->outputs[Mixer::LEFT_OUTPUT].value;
    float outR = m->outputs[Mixer::RIGHT_OUTPUT].value;
    float expectedOutL = side ? float(10 * .8 * .8) : 0;
    float expectedOutR = side ? 0 : float(10 * .8 * .8);
    assertClose(outL, expectedOutL, .01);
    assertClose(outR, expectedOutR, .01);
}


static void testMaster()
{
    testMaster(false);
    testMaster(true);
}

void testMute()
{
    MixerPtr m = getMixer();

    m->inputs[Mixer::AUDIO0_INPUT].value = 10;
    m->params[Mixer::PAN0_PARAM].value = -1.f;     // full left
    m->params[Mixer::MUTE0_PARAM].value = 1;        // mute
    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }
   
    assertClose(m->outputs[Mixer::LEFT_OUTPUT].value, 0, .001);
    assertClose(m->outputs[Mixer::RIGHT_OUTPUT].value, 0, .001);
}


void testSolo()
{
    MixerPtr m = getMixer();

    m->inputs[Mixer::AUDIO0_INPUT].value = 10;
    m->params[Mixer::PAN0_PARAM].value = -1.f;     // full left
    m->params[Mixer::SOLO0_PARAM].value = 1;        // mute
    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }

    assertClose(m->outputs[Mixer::LEFT_OUTPUT].value, float(10 * .8 * .8), .001);
    assertClose(m->outputs[Mixer::RIGHT_OUTPUT].value, 0, .001);
}


static void testPanLook0()
{
    assert(ObjectCache<float>::getMixerPanL());
    assert(ObjectCache<float>::getMixerPanR());
}

#if 1
static inline float _PanL(float balance, float cv)
{ // -1...+1
    float p, inp;
    inp = balance + cv / 5;
    p = M_PI * (std::clamp(inp, -1.0f, 1.0f) + 1) / 4;
    return ::cos(p);
}

static inline float _PanR(float balance, float cv)
{
    float p, inp;
    inp = balance + cv / 5;
    p = M_PI * (std::clamp(inp, -1.0f, 1.0f) + 1) / 4;
    return ::sin(p);
}
#endif

static void testPanLookL()
{
    auto p = ObjectCache<float>::getMixerPanL();
    auto r = ObjectCache<float>::getMixerPanR();
    for (float x = -1; x <= 1; x += .021f) {
        const float lookL = LookupTable<float>::lookup(*p, x);
        const float actualL = _PanL(x, 0);
        assertClose(lookL, actualL, .01);

        const float lookR = LookupTable<float>::lookup(*r, x);
        const float actualR = _PanR(x, 0);
        assertClose(lookR, actualR, .01);

        assertNE(actualL, actualR);
    }
}

void testMix8()
{
    testChannel();
    testMaster();
    testMute();
    testSolo();
    testPanLook0();
    testPanLookL();
}