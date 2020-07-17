
#pragma once

#include "AudioMath.h"
#include "Chaos.h"
#include "Divider.h"
#include "FractionalDelay.h"
#include "IComposite.h"
#include "ObjectCache.h"

#include <assert.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class ChaosKittyDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class ChaosKitty : public TBase
{
public:

    ChaosKitty(Module * module) : TBase(module)
    {
    }
    ChaosKitty() : TBase()
    {
    }

    static std::vector<std::string> typeLabels() {
        return { "noise", "pitched", "circle" };
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds
    {
        CHAOS_PARAM,
        CHAOS_TRIM_PARAM,
        CHAOS2_PARAM,
        CHAOS2_TRIM_PARAM,
        OCTAVE_PARAM,
        TYPE_PARAM,
        BRIGHTNESS_PARAM,
        RESONANCE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CHAOS_INPUT,
        CHAOS2_INPUT,
        V8_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<ChaosKittyDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    void onSampleRateChange(float rate, float time);

private:
    enum class Types { SimpleChaoticNoise, ResonantNoise, Circle};
    Types type = Types::SimpleChaoticNoise;
    SimpleChaoticNoise simpleChaoticNoise;
    ResonantNoise resonantNoise;
    AudioMath::ScaleFun<float> scaleChaos;

    CircleMap circleMap;
    AudioMath::ScaleFun<float> scaleKCircleMap;

    Divider divn;
    Divider divm;
    void stepn(int);
    void stepm(int);
    void updatePitch();
    float gainAdjust = 1;

     std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
};

template <class TBase>
inline void ChaosKitty<TBase>::onSampleRateChange(float rate, float time)
{
    assert(rate > 10000);
    assert(time < .1);
    simpleChaoticNoise.onSampleRateChange(rate, time);
    resonantNoise.onSampleRateChange(rate, time);
    circleMap.onSampleRateChange(rate, time);
}

template <class TBase>
inline void ChaosKitty<TBase>::init()
{
    divn.setup(4, [this] {
        this->stepn(divn.getDiv());
    });
    divm.setup(16, [this] {
        this->stepm(divm.getDiv());
    });

    scaleChaos = AudioMath::makeLinearScaler<float>(3.5, 4);
    scaleKCircleMap = AudioMath::makeLinearScaler<float>(1.2, 75);
}

template <class TBase>
inline void ChaosKitty<TBase>::stepm(int n) {
    if (type == Types::SimpleChaoticNoise) {
        gainAdjust = 5.f / 3.5f;
    } else if (type == Types::ResonantNoise) {
        gainAdjust = 5.f / 2.f;
    } else if (type == Types::Circle) {
        gainAdjust = 1;
    }else {
        gainAdjust = 0;
       // assert(false);
    }
}

template <class TBase>
inline void ChaosKitty<TBase>::stepn(int n) {
    type = Types(int(std::round(TBase::params[TYPE_PARAM].value)));

    const float chaosCV = TBase::inputs[CHAOS_INPUT].getVoltage(0) / 10.f;
    const float g  = scaleChaos(
        chaosCV,
        TBase::params[CHAOS_PARAM].value,
        TBase::params[CHAOS_TRIM_PARAM].value);
    simpleChaoticNoise.setG(g);
    resonantNoise.setG(g);
    
    float x = scaleKCircleMap(
        chaosCV,
        TBase::params[CHAOS_PARAM].value,
        TBase::params[CHAOS_TRIM_PARAM].value);   
    circleMap.setChaos(x);

    updatePitch();
}

template <class TBase>
inline void ChaosKitty<TBase>::step()
{
    divm.step();
    divn.step();
    float output = gainAdjust;
    if (type == Types::SimpleChaoticNoise) {
        output *= simpleChaoticNoise.step();
    } else if (type == Types::ResonantNoise) {
        output *= resonantNoise.step();
    } else if (type == Types::Circle) {
        output *= circleMap.step();
    }else {
        output = 0;
    }

    output = std::min(output, 8.f);
    output = std::max(output, -8.f);
    TBase::outputs[MAIN_OUTPUT].setVoltage(output, 0);

  
}

template <class TBase>
inline void ChaosKitty<TBase>::updatePitch()
{
    float pitch = 1.0f + roundf(TBase::params[OCTAVE_PARAM].value); // + TBase::params[TUNE_PARAM].value / 12.0f;
    pitch += TBase::inputs[V8_INPUT].getVoltage(0);

    const float q = float(std::log2(261.626));       // move up to pitch range of even vco
    pitch += q;
    const float _freq = expLookup(pitch);
    #if 0
    {
        static float lastFreq;
        if (_freq != lastFreq) {
            printf("will now set to freq %f\n", _freq); fflush(stdout);
            lastFreq = _freq;
        }
    }
    #endif

    float brightness = TBase::params[BRIGHTNESS_PARAM].value;
    float resonance = TBase::params[RESONANCE_PARAM].value;
    resonantNoise.set(_freq, brightness, resonance);

    circleMap.setFreq(_freq);
}

template <class TBase>
int ChaosKittyDescription<TBase>::getNumParams()
{
    return ChaosKitty<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config ChaosKittyDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case ChaosKitty<TBase>::CHAOS_PARAM:
            ret = {-5, 5, 0, "Chaos"};
            break;
        case ChaosKitty<TBase>::CHAOS2_PARAM:
            ret = { -5, 5, 0, "Chaos 2" };
            break;
        case ChaosKitty<TBase>::CHAOS_TRIM_PARAM:
            ret = {-1, 1, 0, "Chaos trim"};
            break;
        case ChaosKitty<TBase>::CHAOS2_TRIM_PARAM:
            ret = { -1, 1, 0, "Chaos 2 trim" };
            break;
        case ChaosKitty<TBase>::TYPE_PARAM:
            ret = { 0, 2, 0, "type" };
            break;
        case ChaosKitty<TBase>::OCTAVE_PARAM:
            ret = {-5, 5, 0, "octave"};
            break;
        case ChaosKitty<TBase>::BRIGHTNESS_PARAM:
            ret = {0, 1, .5, "brightness"};
            break;
        case ChaosKitty<TBase>::RESONANCE_PARAM:
            ret = {0, 1, .5, "resonance"};
            break;
        default:
            assert(false);
    }
    return ret;
}


