
#include "asserts.h"
#include "Mix8.h"
#include "Mix4.h"
#include "MixM.h"
#include "ObjectCache.h"
#include "TestComposite.h"

using Mixer8 = Mix8<TestComposite>;
using Mixer4 = Mix4<TestComposite>;
using MixerM = MixM<TestComposite>;

template <typename T>
static std::shared_ptr<T> getMixer()
{
    auto ret = std::make_shared<T>();
    ret->init();
    auto icomp = ret->getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        ret->params[i].value = param.def;
    }
    return ret;
}

template <typename T>
void testChannel(int channel, bool useParam)
{
    T m;
    m.init();

    const float activeParamValue = useParam ? 2.f : 1.f;
    const float activeCVValue = useParam ? 5.f : 10.f;

    // zero all inputs, put all channel gains to 1
    for (int i = 0; i < T::numChannels; ++i) {
        m.inputs[T::AUDIO0_INPUT + i].value = 0;
        m.params[T::GAIN0_PARAM + i].value = 1;
    }

    auto xx = m.inputs[T::PAN0_INPUT].value;
    auto yy = m.params[T::PAN0_PARAM].value;

    m.inputs[T::AUDIO0_INPUT + channel].value = 5.5f;
    m.params[T::GAIN0_PARAM + channel].value = activeParamValue;
    m.inputs[T::LEVEL0_INPUT + channel].value = activeCVValue;
    m.inputs[T::LEVEL0_INPUT + channel].active = true;

    for (int i = 0; i < 1000; ++i) {
        m.step();           // let mutes settle
    }

    for (int i = 0; i < T::numChannels; ++i) {
        float expected = (i == channel) ? 5.5f : 0;
        assertClose(m.outputs[T::CHANNEL0_OUTPUT + i].value, expected, .01f);
    }
}

template <typename T>
static void testChannel()
{
    for (int i = 0; i < T::numChannels; ++i) {
        testChannel<T>(i, true);
        testChannel<T>(i, false);
    }
}

template <typename T>
static void testMaster(bool side)
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].value = 10;
    m->params[T::PAN0_PARAM].value = side ? -1.f : 1.f;     // full left

    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }
    float outL = m->outputs[T::LEFT_OUTPUT].value;
    float outR = m->outputs[T::RIGHT_OUTPUT].value;
    float expectedOutL = side ? float(10 * .8 * .8) : 0;
    float expectedOutR = side ? 0 : float(10 * .8 * .8);
    assertClose(outL, expectedOutL, .01);
    assertClose(outR, expectedOutR, .01);
}

template <typename T>
static void testMaster()
{
    testMaster<T>(false);
    testMaster<T>(true);
}

template <typename T>
void testMute()
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].value = 10;
    m->params[T::PAN0_PARAM].value = -1.f;     // full left
    m->params[T::MUTE0_PARAM].value = 1;        // mute
    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }
   
    assertClose(m->outputs[T::LEFT_OUTPUT].value, 0, .001);
    assertClose(m->outputs[T::RIGHT_OUTPUT].value, 0, .001);

    // un-mute
    m->params[T::MUTE0_PARAM].value = 0;    
    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }
    assertGT(m->outputs[T::LEFT_OUTPUT].value, 5);
   // assertGT(m->outputs[T::RIGHT_OUTPUT].value, 5);

    m->inputs[T::MUTE0_INPUT].value = 10;       //mute with CV

    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }

    assertClose(m->outputs[T::LEFT_OUTPUT].value, 0, .001);
    assertClose(m->outputs[T::RIGHT_OUTPUT].value, 0, .001);

}

template <typename T>
void testSolo()
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].value = 10;
    m->params[T::PAN0_PARAM].value = -1.f;     // full left
    m->params[T::SOLO0_PARAM].value = 1;        // mute
    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }

    assertClose(m->outputs[T::LEFT_OUTPUT].value, float(10 * .8 * .8), .001);
    assertClose(m->outputs[T::RIGHT_OUTPUT].value, 0, .001);
}

static void testPanLook0()
{
    assert(ObjectCache<float>::getMixerPanL());
    assert(ObjectCache<float>::getMixerPanR());
}

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


template <typename T>
static void testPanMiddle()
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].value = 10;
    m->params[T::PAN0_PARAM].value = 0;     // pan in middle

    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }
    float outL = m->outputs[T::LEFT_OUTPUT].value;
    float outR = m->outputs[T::RIGHT_OUTPUT].value;
    float expectedOut = float(10 * .8 * .8 / sqrt(2.f));
  
    assertClose(outL, expectedOut, .01);
    assertClose(outR, expectedOut, .01);
}


template <typename T>
static void testMasterMute()
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].value = 10;
    m->params[T::PAN0_PARAM].value = 0;     // straight up
    m->params[T::MASTER_MUTE_PARAM].value = 1;

    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }
    float outL = m->outputs[T::LEFT_OUTPUT].value;
    float outR = m->outputs[T::RIGHT_OUTPUT].value;
    float expectedOut = 0;
    assertClose(outL, expectedOut, .01);
    assertClose(outR, expectedOut, .01);
}

#include "ExtremeTester.h"
template <typename T>
static void testInputExtremes()
{
    T dut;
   // dut.setSampleRate(44100);

    std::vector< std::pair<float, float> > paramLimits;
    dut.init();

    paramLimits.resize(dut.NUM_PARAMS);
    using fp = std::pair<float, float>;
   

    auto iComp = T::getDescription();
    for (int i = 0; i < iComp->getNumParams(); ++i) {
        auto desc = iComp->getParam(i);
        fp t(desc.min, desc.max);
        paramLimits[i] = t;
    }

    ExtremeTester<T>::test(dut, paramLimits, false, "mix");
}


static void testExpansion4()
{
    float inbuf[6];
    float outbuf[6];

    auto m = getMixer<Mixer4>();
    inbuf[0] = 1.1f;
    inbuf[1] = 3.2f;
    m->setExpansionInputs(inbuf);
    m->setExpansionOutputs(outbuf);
    m->step();
    assertEQ(outbuf[0], 1.1f);
    assertEQ(outbuf[1], 3.2f);

    // output,no input
    m->setExpansionInputs(nullptr);
    m->step();
    assertEQ(outbuf[0], 0);
    assertEQ(outbuf[1], 0);

    // input, no output
    m->setExpansionInputs(inbuf);
    m->setExpansionOutputs(nullptr);
    m->step();
    assertEQ(outbuf[0], 0);
    assertEQ(outbuf[1], 0);
}


static void testExpansionM()
{
    float inbuf[6];

    auto m = getMixer<MixerM>();
    inbuf[0] = 1.1f;
    inbuf[1] = 3.2f;
    m->setExpansionInputs(inbuf);

    m->params[MixerM::MASTER_VOLUME_PARAM].value = 1;

    for (int i = 0; i < 1000; ++i) {
        m->step();           // let mutes settle
    }

    assertClose(m->outputs[MixerM::LEFT_OUTPUT].value, 1.1f, .01);
    assertClose(m->outputs[MixerM::RIGHT_OUTPUT].value, 3.2f, .01);

    // disconnect input
    m->setExpansionInputs(nullptr);
    m->step();
   
    assertClose(m->outputs[MixerM::LEFT_OUTPUT].value, 0, .01);
    assertClose(m->outputs[MixerM::RIGHT_OUTPUT].value, 0, .01);
}



void testMix8()
{
    testChannel<Mixer8>();
    testChannel<Mixer4>();
    testChannel<MixerM>();

    testMaster<Mixer8>();
    testMaster<MixerM>();

  //  testMute<Mixer8>();
    testMute<MixerM>();
    // TODO test 4 also
    testSolo<Mixer8>();
    testSolo<MixerM>();
    testPanLook0();
    testPanLookL();

    testPanMiddle<Mixer8>();
    testPanMiddle<MixerM>();
    testMasterMute<Mixer8>();
    testMasterMute<MixerM>();

    testExpansion4();
    testExpansionM();

#if 0 // these take too long
    testInputExtremes<Mixer8>();
    testInputExtremes<MixerM>();
    testInputExtremes<Mixer4>();
#endif
}