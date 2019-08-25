
#include "Mix4.h"
#include "MixStereo.h"

#include "TestComposite.h"
#include "asserts.h"

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
    for (int i = 0; i < 10; ++i) {
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
static void _testMaster(std::function<float(std::shared_ptr<T>, bool bRight)> outputGetter, bool side)
{
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



void testStereoMix()
{
    testChannel<MixerS>();
    
    testExpansionPassthrough<Mixer4>(outputGetterMix4, outputSenderMix4);
    testExpansionPassthrough<Mixer4>(auxGetterMix4, auxSenderMix4);
    testExpansionPassthrough<Mixer4>(auxGetterMix4B, auxSenderMix4B);
    testExpansionPassthrough<MixerS>(outputGetterMixS, outputSenderMixS);
    testExpansionPassthrough<MixerS>(auxGetterMixS, auxSenderMixS);
    testExpansionPassthrough<MixerS>(auxGetterMixSB, auxSenderMixSB);
}