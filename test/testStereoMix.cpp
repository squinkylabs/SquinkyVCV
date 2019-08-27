
#include "Mix4.h"
#include "MixStereo.h"

#include "TestComposite.h"
#include "asserts.h"

#include <memory>
#include <memory>

const static float defaultMasterGain = 1.002374f;

using MixerS = MixStereo<TestComposite>;
using Mixer4 = Mix4<TestComposite>;

static float gOutputBuffer[8];
static float gInputBuffer[8];

static void clear()
{
    for (int i = 0; i < 8; ++i) {
        gOutputBuffer[i] = 0;
        gInputBuffer[i] = 0;
    }
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

    return ret;
}

// factory for test mixers
template <typename T>
static std::shared_ptr<T> getMixer();

template <>
std::shared_ptr<MixerS> getMixer<MixerS>()
{
    std::shared_ptr<MixerS> ret = getMixerBase<MixerS>();
    ret->setExpansionOutputs(gOutputBuffer);
    ret->setExpansionInputs(gInputBuffer);
    return ret;
}

template <>
std::shared_ptr<Mixer4> getMixer<Mixer4>()
{
    std::shared_ptr<Mixer4> ret = getMixerBase<Mixer4>();
    ret->setExpansionOutputs(gOutputBuffer);
    ret->setExpansionInputs(gInputBuffer);
    return ret;
}


template <typename T>
static void step(std::shared_ptr<T> mixer)
{
    for (int i = 0; i < 20; ++i) {
        mixer->step();
    }
}

template <typename T>
static void dumpUb(std::shared_ptr<T> mixer)
{
    const int numUb = sizeof(mixer->unbufferedCV) / sizeof(mixer->unbufferedCV[0]);
    printf("ubcv: ");

    int x = 0;
    for (int i = 0; i < numUb; ++i) {
        printf("%.2f ", mixer->unbufferedCV[i]);
        if (++x >= 4) {
            printf("\n      ");
            x = 0;
        }
    }
    printf("\n");
}


template <typename T>
static void dumpOut(std::shared_ptr<T> mixer)
{
    printf("channel outs:\n");

    int x = 0;
    for (int i = 0; i < T::numChannels; ++i) {
        printf("%.2f ", mixer->outputs[i + T::CHANNEL0_OUTPUT].value);
    }
    printf("\n");
}


static float outputGetterMix4(std::shared_ptr<Mixer4> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return defaultMasterGain * gOutputBuffer[bRight ? 1 : 0];
}

static float outputGetterMixS(std::shared_ptr<MixerS> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return defaultMasterGain * gOutputBuffer[bRight ? 1 : 0];
}

static void outputSenderMix4(std::shared_ptr<Mixer4> m, bool bRight, float value)
{
    // use the expander bus, and apply the default master gain
    //return defaultMasterGain * gOutputBuffer[bRight ? 1 : 0];
    gInputBuffer[bRight ? 1 : 0] = value;
}

static void outputSenderMixS(std::shared_ptr<MixerS> m, bool bRight, float value)
{
    // use the expander bus, and apply the default master gain
    //return defaultMasterGain * gOutputBuffer[bRight ? 1 : 0];
    gInputBuffer[bRight ? 1 : 0] = value;
}

static float auxGetterMix4(std::shared_ptr<Mixer4> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return gOutputBuffer[bRight ? 3 : 2];
}

static float auxGetterMixS(std::shared_ptr<MixerS> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return gOutputBuffer[bRight ? 3 : 2];
}

static void auxSenderMix4(std::shared_ptr<Mixer4> m, bool bRight, float value)
{
    // use the expander bus, and apply the default master gain
    gInputBuffer[bRight ? 3 : 2] = value;
}

static void auxSenderMixS(std::shared_ptr<MixerS> m, bool bRight, float value)
{
    // use the expander bus, and apply the default master gain
    gInputBuffer[bRight ? 3 : 2] = value;
}

static float auxGetterMix4B(std::shared_ptr<Mixer4> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return gOutputBuffer[bRight ? 5 : 4];
}

static float auxGetterMixSB(std::shared_ptr<MixerS> m, bool bRight)
{
    // use the expander bus, and apply the default master gain
    return gOutputBuffer[bRight ? 5 : 4];
}
static void auxSenderMix4B(std::shared_ptr<Mixer4> m, bool bRight, float value)
{
    // use the expander bus, and apply the default master gain
    gInputBuffer[bRight ? 5 : 4] = value;
}
static void auxSenderMixSB(std::shared_ptr<MixerS> m, bool bRight, float value)
{
    // use the expander bus, and apply the default master gain
    gInputBuffer[bRight ? 5 : 4] = value;
}
//***********************************************************************************


template <typename T>
void testChannel(int group, bool useParam)
{
    assert(group < T::numGroups);
    //printf("\n** running test group %d useParam %d\n", 0, useParam);
    auto mixer = getMixer<T>();

    const float activeParamValue = useParam ? 1.f : .25f;
    const float activeCVValue = useParam ? 5.f : 10.f;

    // zero all inputs, put all channel gains to 1
    for (int i = 0; i < T::numChannels; ++i) {
        mixer->inputs[T::AUDIO0_INPUT + i].value = 0;
        mixer->params[T::GAIN0_PARAM + i].value = 1;
    }

    // TODO: make left and right inputs different

    const int leftChannel = group * 2;
    const int rightChannel = 1 + group * 2;

    mixer->inputs[T::AUDIO0_INPUT + leftChannel].value = 5.5f;
    mixer->inputs[T::AUDIO0_INPUT + rightChannel].value = 6.5f;
    mixer->params[T::GAIN0_PARAM + group].value = activeParamValue;
    mixer->inputs[T::LEVEL0_INPUT + group].value = activeCVValue;
    mixer->inputs[T::LEVEL0_INPUT + group].active = true;

    for (int i = 0; i < 10; ++i) {
        mixer->step();           // let mutes settle
    }

    float atten18Db = 1.0f / (2.0f * 2.0f * 2.0f);

    const float exectedInActiveChannelLeft = (useParam) ?
        (5.5f * .5f) :          // param at 1, cv at 5, gain = .5
        atten18Db * 5.5f;       // param at .25 is 18db down cv at 10 is units

    const float exectedInActiveChannelRight = (useParam) ?
        (6.5f * .5f) :          // param at 1, cv at 5, gain = .5
        atten18Db * 6.5f;       // param at .25 is 18db down cv at 10 is units

    //dumpUb(mixer);
    //dumpOut(mixer);

    for (int gp = 0; gp < T::numGroups; ++gp) {
        const int leftChannel = gp * 2;
        const int rightChannel = 1 + gp * 2;
       
        float expectedLeft = (gp == group) ? exectedInActiveChannelLeft : 0;
        float expectedRight = (gp == group) ? exectedInActiveChannelRight : 0;
          
        assertClose(mixer->outputs[leftChannel].value, expectedLeft, .01f);
        assertClose(mixer->outputs[rightChannel].value, expectedRight, .01f);
    }
}

template <typename T>
static void testChannel()
{
    for (int i = 0; i < T::numGroups; ++i) {
        testChannel<T>(i, true);
        testChannel<T>(i, false);
    }
}

template <typename T>
static void _testExpansionPassthrough(
    std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter,
    std::function<void(std::shared_ptr<T>, bool bRight, float)> inputPutter,
    bool bRight,
    float testValue
)
{
    auto mixer = getMixer<T>();
    clear();
    inputPutter(mixer, bRight, testValue);

    step(mixer);

    const float x = outputGetter(mixer, bRight);
    assertClose(x, testValue, .01);
}

template <typename T>
static void testExpansionPassthrough(
    std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter,
    std::function<void(std::shared_ptr<T>, bool bRight, float)> inputPutter
    )
{
    _testExpansionPassthrough(outputGetter, inputPutter, true, .33f);
    _testExpansionPassthrough(outputGetter, inputPutter, false, .87f);
}

template <typename T>
static void _testMaster(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter, bool side, int group)
{    
    const int channel = 2 * group;
    auto m = getMixer<T>();

    // put input in both left and right
    m->inputs[T::AUDIO0_INPUT + channel].value = 7.7f;
    m->inputs[T::AUDIO0_INPUT + channel + 1].value = 7.7f;

    m->params[T::GAIN0_PARAM + group].value = 1;
    m->params[T::PAN0_PARAM + group].value = side ? -1.f : 1.f;     // full left or right

    step(m);
    m->step();

    float outL = outputGetter(m, 0);
    float outR = outputGetter(m, 1);

    // input 10, channel atten 1, master default .8
    float expectedOutL = side ? float(7.7f * defaultMasterGain) : 0;
    float expectedOutR = side ? 0 : float(7.7f * defaultMasterGain);
    assertClose(outL, expectedOutL, .01);
    assertClose(outR, expectedOutR, .01);
}

template <typename T>
static void testMaster(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter)
{
    _testMaster<T>(outputGetter, false, 0);
    _testMaster<T>(outputGetter, true, 0);
    _testMaster<T>(outputGetter, false, 1);
    _testMaster<T>(outputGetter, true, 1);
}

template <typename T>
void _testMaster2(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter, int group, bool side)
{
    int channel = 2 * group;
    if (side) {
        ++channel;
    }
    auto m = getMixer<T>();



    m->inputs[T::AUDIO0_INPUT + channel].value = 1.3f;
    m->params[T::GAIN0_PARAM + group].value = 1;
    m->params[T::PAN0_PARAM + group].value = 0;     // middle

    step(m);

    float outL = outputGetter(m, 0);
    float outR = outputGetter(m, 1);

    float expectedOutL = side ? 0 : 1.3f;
    float expectedOutR = side ? 1.3f : 0;
    assertClose(outL, expectedOutL, .01);
    assertClose(outR, expectedOutR, .01);
}

template <typename T>
void _testAuxOutA(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter, int group, bool side)
{
    int channel = 2 * group;
    auto m = getMixer<T>();

    const float testValue = 3.3f;
    m->inputs[T::AUDIO0_INPUT + channel].value = side ? 0 : testValue;
    m->inputs[T::AUDIO0_INPUT + channel + 1].value = side ? testValue : 0;
    m->params[T::GAIN0_PARAM + group].value = 1;
    m->params[T::SEND0_PARAM + group].value = 1;
    m->params[T::PAN0_PARAM + group].value = 0;     // middle

    step(m);

    float outL = outputGetter(m, 0);
    float outR = outputGetter(m, 1);

    float expectedOutL = side ? 0 : testValue;
    float expectedOutR = side ? testValue : 0;
    assertClose(outL, expectedOutL, .01);
    assertClose(outR, expectedOutR, .01);
}

template <typename T>
static void testMaster2(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter)
{
    _testMaster2<T>(outputGetter, false, 0);
    _testMaster2<T>(outputGetter, true, 0);
    _testMaster2<T>(outputGetter, false, 1);
    _testMaster2<T>(outputGetter, true, 1);
}

template <typename T>
static void testAuxOutA(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter)
{
    _testAuxOutA<T>(outputGetter, false, 0);
    _testAuxOutA<T>(outputGetter, true, 0);
    _testAuxOutA<T>(outputGetter, false, 1);
    _testAuxOutA<T>(outputGetter, true, 1);
}

template <typename T>
void _testMute(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter, int group, bool bRight)
{
    const int channel = group * 2;
    auto m = getMixer<T>();
    m->step();          // let mutes see zero first (startup reset)

   
    // put 10 in both sides
    m->inputs[T::AUDIO0_INPUT + channel].value = 10;
    m->inputs[T::AUDIO0_INPUT + channel + 1].value = 10;
    m->params[T::GAIN0_PARAM + group].value = 1;

    // pan to extremes
    m->params[T::PAN0_PARAM + group].value = bRight ? 1.f : -1.f;  

    // now mute input 0
    m->params[T::MUTE0_PARAM + group].value = 1;        // mute
    step(m);

    assertClose(outputGetter(m, bRight), 0, .001);

    m->stepn(4);    // let's watch it update the light
    const int xx = T::MUTE0_LIGHT + group;
    const float yy = m->lights[T::MUTE0_LIGHT + group].value;
    assertGT(m->lights[T::MUTE0_LIGHT + group].value, 5.f);

    // un-mute
    m->params[T::MUTE0_PARAM + group].value = 0;
    step(m);
    m->params[T::MUTE0_PARAM + group].value = 1;
    step(m);

    float s1 = outputGetter(m, bRight);
    assertGT(s1, 5);
    assertLT(m->lights[T::MUTE0_LIGHT + group].value, 5.f);

    m->inputs[T::MUTE0_INPUT + group].value = 10;       //mute with CV

    step(m);

    assertClose(outputGetter(m, bRight), 0, .01);
    assertGT(m->lights[T::MUTE0_LIGHT + group].value, 5.f);
}

template <typename T>
void testMute(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter)
{
    _testMute(outputGetter, 0, false);
    _testMute(outputGetter, 0, true);
    _testMute(outputGetter, 1, false);
    _testMute(outputGetter, 1, true);
}


//**********************************************


/**
 * param augGetter is one of the functions that will retrieve data from the aux send.
 * param side is true if left,  false if right
 * param aux0 is true if we want to test the aux0 bus, false for aux1
 * param sendParam is the parameter id for the send level
 * param pre is true for pre-fader send
 * param preParam is (redundant?)
 * 
 */
template <typename T>
static void _testAuxOut(
    std::function<float(std::shared_ptr<T>, bool bRight)> auxGetter,
    bool side,
    bool aux0,
    int sendParam,
    bool pre,
    int preParam,
    int group
)
{
    printf("\n** textAuxOut side=%d, group=%d\n", side, group);
    auto m = getMixer<T>();
    const int channel = group * 2;

    m->inputs[T::AUDIO0_INPUT + channel].value = 10;
    m->params[T::GAIN0_PARAM + group].value = 1;

    m->params[T::PAN0_PARAM + group].value = side ? -1.f : 1.f;     // full side
    m->params[sendParam ].value = 1;

    if (preParam) {
        assert(false);      // finish me?
        m->params[preParam].value = pre ? 1.f : 0.f;
    }

    // with pre-fader, should still get out with no volume
    // TODO: test that with post fade the fader has an effect
    if (pre) {
        assert(false);
        m->params[T::GAIN0_PARAM].value = 0;
    }

    step(m);
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

    dumpUb(m);
    dumpOut(m);

    assertClose(auxL, expectedOutL, .01);
    assertClose(auxR, expectedOutR, .01);
}

template <typename T>
static void testAuxOut(std::function<float(std::shared_ptr<T>, bool bRight)> auxGetter)
{
    _testAuxOut<T>(auxGetter, false, true, T::SEND0_PARAM, false, 0, 0);
    _testAuxOut<T>(auxGetter, true, true, T::SEND0_PARAM, false, 0, 0);

    _testAuxOut<T>(auxGetter, false, true, T::SEND0_PARAM, false, 0, 1);
}

void testStereoMix()
{
    testChannel<MixerS>();
    
    testExpansionPassthrough<Mixer4>(outputGetterMix4, outputSenderMix4);
    testExpansionPassthrough<Mixer4>(auxGetterMix4, auxSenderMix4);
    testExpansionPassthrough<Mixer4>(auxGetterMix4B, auxSenderMix4B);
    testExpansionPassthrough<MixerS>(outputGetterMixS, outputSenderMixS);
    testExpansionPassthrough<MixerS>(auxGetterMixS, auxSenderMixS);
    testExpansionPassthrough<MixerS>(auxGetterMixSB, auxSenderMixSB);

    testMaster2<MixerS>(outputGetterMixS);
    testMaster<MixerS>(outputGetterMixS);
    testMute<MixerS>(outputGetterMixS);
    testAuxOutA<MixerS>(auxGetterMixS);
}