
#include "TestComposite.h"
#include "asserts.h"
#include "Mix8.h"
#include "Mix4.h"
#include "MixM.h"
#include "ObjectCache.h"


const static float defaultMasterGain = 1.002374f;

using Mixer4 = Mix4<TestComposite>;
using MixerM = MixM<TestComposite>;

static float gOutputBuffer[8];


template <typename T>
static void step(std::shared_ptr<T> mixer)
{
    for (int i = 0; i < 20; ++i) {
        mixer->step();
    }
}

// function that knows how to get left output from a mixerM
static float outputGetterMixM(std::shared_ptr<MixerM> m, bool bRight)
{
    return m->outputs[bRight ? MixerM::RIGHT_OUTPUT : MixerM::LEFT_OUTPUT].getVoltage(0);
}

static float auxGetterMixM(std::shared_ptr<MixerM> m, bool bRight)
{
    return m->outputs[bRight ? MixerM::RIGHT_SEND_OUTPUT : MixerM::LEFT_SEND_OUTPUT].getVoltage(0);
}

static float auxGetterMixMB(std::shared_ptr<MixerM> m, bool bRight)
{
    return m->outputs[bRight ? MixerM::RIGHT_SENDb_OUTPUT : MixerM::LEFT_SENDb_OUTPUT].getVoltage(0);
}

static float outputGetterMix4(std::shared_ptr<Mixer4> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return defaultMasterGain * gOutputBuffer[bRight ? 1 : 0];
}

static float auxGetterMix4(std::shared_ptr<Mixer4> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return gOutputBuffer[bRight ? 3 : 2];
}

static float auxGetterMix4B(std::shared_ptr<Mixer4> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return gOutputBuffer[bRight ? 5 : 4];
}


template <typename T>
static std::shared_ptr<T> getMixerBase()
{
    auto ret = std::make_shared<T>();
    ret->init();
    auto icomp = ret->getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        ret->params[i].value = param.def;
    }
    ret->_disableAntiPop();

    // default to monophonic inputs
    for (int i = 0; i < T::numChannels; ++i) {
        ret->inputs[T::AUDIO0_INPUT + i].channels = 1;
    }

    return ret;
}

// factory for test mixers
template <typename T>
static std::shared_ptr<T> getMixer();

template <>
std::shared_ptr<Mixer4> getMixer<Mixer4>()
{
    std::shared_ptr<Mixer4> ret = getMixerBase<Mixer4>();
    ret->setExpansionOutputs(gOutputBuffer);
    return ret;
}

template <>
std::shared_ptr<MixerM> getMixer<MixerM>()
{
    std::shared_ptr<MixerM> ret = getMixerBase<MixerM>();
    return ret;
}


//*****************************************************************************


template <typename T>
void testChannel(int channel, bool useParam)
{
   // T m;
   // m.init();
    auto m = getMixer<T>();

    // param > 1 is illegal. Fix this test!
    const float activeParamValue = useParam ? 1.f : .25f;
    const float activeCVValue = useParam ? 5.f : 10.f;

    // zero all inputs, put all channel gains to 1
    for (int i = 0; i < T::numChannels; ++i) {
        m->inputs[T::AUDIO0_INPUT + i].setVoltage(0, 0);
        m->params[T::GAIN0_PARAM + i].value = 1;
    }

    m->inputs[T::AUDIO0_INPUT + channel].setVoltage(5.5f, 0);
    m->params[T::GAIN0_PARAM + channel].value = activeParamValue;
    m->inputs[T::LEVEL0_INPUT + channel].setVoltage(activeCVValue, 0);
    m->inputs[T::LEVEL0_INPUT + channel].channels = 1;

    step(m);

    float atten18Db = 1.0f / (2.0f * 2.0f * 2.0f);  

    const float exectedInActiveChannel = (useParam) ? 
        (5.5f * .5f) :       // param at 1, cv at 5, gain = .5
        atten18Db * 5.5f;       // param at .25 is 18db down cv at 10 is units

    for (int i = 0; i < T::numChannels; ++i) {

       // auto debugMuteState = m.params[T::MUTE0_STATE_PARAM + i];
        float expected = (i == channel) ? exectedInActiveChannel : 0;
        assertClose(m->outputs[T::CHANNEL0_OUTPUT + i].getVoltage(0), expected, .01f);
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

//***********************************************************************************

template <typename T>
static void _testMaster(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter, bool side)
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].setVoltage(10, 0);
    m->params[T::GAIN0_PARAM].value = 1;
    m->params[T::PAN0_PARAM].value = side ? -1.f : 1.f;     // full left

    step(m);

    float outL = outputGetter(m, 0);
    float outR = outputGetter(m, 1);

    // input 10, channel atten 1, master default .8
    float expectedOutL = side ? float(10 * defaultMasterGain) : 0;
    float expectedOutR = side ? 0 : float(10 * defaultMasterGain);
    assertClose(outL, expectedOutL, .01);
    assertClose(outR, expectedOutR, .01);
}

template <typename T>
static void testMaster(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter)
{
    _testMaster<T>(outputGetter, false);
    _testMaster<T>(outputGetter, true);
}

//****************************************************************************


template <typename T>
void testMute(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter)
{
    auto m = getMixer<T>();
    m->step();          // let mutes see zero first (startup reset)

    // now mute input 0
    m->inputs[T::AUDIO0_INPUT].setVoltage(10, 0);
    m->params[T::GAIN0_PARAM].value = 1;
    m->params[T::PAN0_PARAM].value = -1.f;     // full left
    m->params[T::MUTE0_PARAM].value = 1;        // mute
    
    step(m);

    assertClose(outputGetter(m, false), 0, .001);
    assertGT(m->lights[T::MUTE0_LIGHT].value, 5.f);

    // un-mute
    m->params[T::MUTE0_PARAM].value = 0;
   
    step(m);


    m->params[T::MUTE0_PARAM].value = 1;
    step(m);

    float s1 = outputGetter(m, false);
    assertGT(s1, 5);
    assertLT(m->lights[T::MUTE0_LIGHT].value, 5.f);

    m->inputs[T::MUTE0_INPUT].setVoltage(10, 0);       //mute with CV

    step(m);

    assertClose(outputGetter(m, false), 0, .001);
    assertGT(m->lights[T::MUTE0_LIGHT].value, 5.f);
}

//**********************************************


/**
 * param augGetter is one of the functions that will retrieve data from the aux send.
 * param side is true if left,  false if right
 * param aux0 is true if we want to test the aux0 bus, false for aux1
 * param sendParam is the parameter id for the send level
 * param pre is true for pre-fader send
 */
template <typename T>
static void _testAuxOut(
    std::function<float(std::shared_ptr<T>, bool bRight)> auxGetter,
    bool side,
    bool aux0,
    int sendParam,
    bool pre,
    int preParam
)
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].setVoltage(10, 0);
    m->params[T::GAIN0_PARAM].value = 1;

    m->params[T::PAN0_PARAM].value = side ? -1.f : 1.f;     // full left
    m->params[sendParam].value = 1;

    if (preParam) {
        m->params[preParam].value = pre ? 1.f : 0.f;
    }

    // with pre-fader, should still get out with no volume
    // TODO: test that with post fade the fader has an effect
    if (pre) {
        m->params[T::GAIN0_PARAM].value = 0;
    }

    step(m);

    float auxL = auxGetter(m, 0);
    float auxR = auxGetter(m, 1);

    float expectedOutL = side ? float(10 * 1) : 0;
    float expectedOutR = side ? 0 : float(10 * 1);
    if (pre) {
        // no pan control on pre, just fixed as if middle all the time.
        expectedOutL = float(10 * (1.f / sqrt(2.f)));
        expectedOutR = float(10 * (1.f / sqrt(2.f)));
    }

    assertClose(auxL, expectedOutL, .01);
    assertClose(auxR, expectedOutR, .01);
}

template <typename T>
static void testAuxOut(std::function<float(std::shared_ptr<T>, bool bRight)> auxGetter)
{
    _testAuxOut<T>(auxGetter, false, true, T::SEND0_PARAM, false, 0);
    _testAuxOut<T>(auxGetter, true, true, T::SEND0_PARAM, false, 0);
}


template <typename T>
static void testAuxOutB(std::function<float(std::shared_ptr<T>, bool bRight)> auxGetter)
{
    _testAuxOut<T>(auxGetter, false, false, T::SENDb0_PARAM, false, 0);
    _testAuxOut<T>(auxGetter, true, false, T::SENDb0_PARAM, false, 0);
}

template <typename T>
static void testAuxOutBpre(std::function<float(std::shared_ptr<T>, bool bRight)> auxGetter)
{
    _testAuxOut<T>(auxGetter, false, false, T::SENDb0_PARAM, true, T::PRE_FADERb_PARAM);
    _testAuxOut<T>(auxGetter, true, false, T::SENDb0_PARAM, true, T::PRE_FADERb_PARAM);
}

template <typename T>
static void testAuxOutApre(std::function<float(std::shared_ptr<T>, bool bRight)> auxGetter)
{
    _testAuxOut<T>(auxGetter, false, false, T::SEND0_PARAM, true, T::PRE_FADERa_PARAM);
    _testAuxOut<T>(auxGetter, true, false, T::SEND0_PARAM, true, T::PRE_FADERa_PARAM);
}


//*********************************************************************


template <typename T>
static void testPanMiddle(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter)
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].setVoltage(10, 0);
    m->params[T::GAIN0_PARAM].value = 1;
    m->params[T::PAN0_PARAM].value = 0;     // pan in middle

    step(m);

    float outL = outputGetter(m, false);
    float outR = outputGetter(m, false);
    float expectedOut = float(10 * 1 * defaultMasterGain / sqrt(2.f));

    assertClose(outL, expectedOut, .01);
    assertClose(outR, expectedOut, .01);
}

//***************************************************************

template <typename T>
void testSoloLegacy(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter)
{
    auto m = getMixer<T>();

    m->inputs[T::AUDIO0_INPUT].setVoltage(10, 0);
    m->params[T::GAIN0_PARAM].value = 1;
    m->params[T::PAN0_PARAM].value = -1.f;     // full left
    m->params[T::SOLO0_PARAM].value = 1;        // solo

    step(m);

    // only testing that signal passes
    assertClose(outputGetter(m, false), float(10 * 1 * defaultMasterGain), .01);
    assertClose(outputGetter(m, true), 0, .01);

    // now solo other channel, should mute all
    m->params[T::SOLO0_PARAM].value = 0;        // solo
    m->params[T::SOLO1_PARAM].value = 1;        // solo

    step(m);

    assertClose(outputGetter(m, false), 0, .01);
    assertClose(outputGetter(m, true), 0, .01);
}

template <typename T>
void testPolyInputSimple(int polyphony, int inputChannel)
{
    auto m = getMixer<T>();

    // zero all inputs, put all channel gains to 1
    for (int i = 0; i < T::numChannels; ++i) {
        m->inputs[T::AUDIO0_INPUT + i].setVoltage(0, 0);
        m->params[T::GAIN0_PARAM + i].value = 1;
    }

    m->inputs[T::AUDIO0_INPUT + inputChannel].channels = polyphony;
    m->inputs[T::AUDIO0_INPUT + inputChannel].voltages[0] = 3.4f;


    step(m);
    const float output = m->outputs[T::CHANNEL0_OUTPUT + inputChannel].voltages[0];
    const float expected = (polyphony == 0) ? 0 : 3.4f / polyphony;

    assertClose(output, expected, .001);
}

template <typename T>
void testPolyInputSimple()
{
    testPolyInputSimple<T>(1, 0);
    testPolyInputSimple<T>(2, 3);
    testPolyInputSimple<T>(3, 2);
    testPolyInputSimple<T>(0, 0);
}

void testMix4()
{
    testChannel<Mixer4>();
    testChannel<MixerM>();

    testMaster<Mixer4>(outputGetterMix4);
    testMaster<MixerM>(outputGetterMixM);

    testMute<Mixer4>(outputGetterMix4);
    testMute<MixerM>(outputGetterMixM);

    testAuxOut<MixerM>(auxGetterMixM);
    testAuxOut<Mixer4>(auxGetterMix4);

    testAuxOutB<MixerM>(auxGetterMixMB);
    testAuxOutB<Mixer4>(auxGetterMix4B);

    testAuxOutBpre<MixerM>(auxGetterMixMB);
    testAuxOutApre<MixerM>(auxGetterMixM);
    testAuxOutBpre<Mixer4>(auxGetterMix4B);
    testAuxOutApre<Mixer4>(auxGetterMix4);

    testPanMiddle<MixerM>(outputGetterMixM);
    testPanMiddle<Mixer4>(outputGetterMix4);

    testSoloLegacy<Mixer4>(outputGetterMix4);
    testSoloLegacy<MixerM>(outputGetterMixM);

    testPolyInputSimple<MixerM>();
    testPolyInputSimple<Mixer4>();
}