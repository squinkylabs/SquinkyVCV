#include "MixStereo.h"

#include "TestComposite.h"
#include "asserts.h"

#include <memory>

using MixerS = MixStereo<TestComposite>;

static float gOutputBuffer[8];

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
    return ret;
}


//***********************************************************************************


template <typename T>
void testChannel(int channel, bool useParam)
{
    T m;
    m.init();

    // param > 1 is illegal. Fix this test!
    const float activeParamValue = useParam ? 1.f : .25f;
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

    float atten18Db = 1.0f / (2.0f * 2.0f * 2.0f);

    const float exectedInActiveChannel = (useParam) ?
        (5.5f * .5f) :       // param at 1, cv at 5, gain = .5
        atten18Db * 5.5f;       // param at .25 is 18db down cv at 10 is units

    for (int i = 0; i < T::numChannels; ++i) {

       // auto debugMuteState = m.params[T::MUTE0_STATE_PARAM + i];
        float expected = (i == channel) ? exectedInActiveChannel : 0;
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

void testStereoMix()
{
    testChannel<MixerS>();
}