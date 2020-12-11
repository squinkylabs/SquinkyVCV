
#pragma once

#include <algorithm>
#include <assert.h>
#include <memory>
#include "IComposite.h"
#include "OscSmoother.h"

#include "simd.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class DividerXDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};


#if 0 // not used?
class HalfBlep
{
public:
    void insertDiscontinuity(float phase, float amp) {
        minBlep.insertDiscontinuity(phase, amp);
        recycle = false;
    }
    float process() {
        float ret = 0;
        if (recycle) {
            ret = last;
            last = 0;
            recycle = false;
        } else {
            ret = minBlep.process();
            last = ret;
            recycle = true;
        }
        return ret;
    }
private:
    dsp::MinBlepGenerator<16, 16, float> minBlep;
    bool recycle;
    float last;
};
#endif

template <class TBase>
class DividerX : public TBase
{
public:

    DividerX(Module * module) : TBase(module)
    {
    }

    DividerX() : TBase()
    {
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
        MINBLEP_PARAM,
        STABILIZER_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        MAIN_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        FIRST_OUTPUT,
        DEBUG_OUTPUT,
        STABILIZER_OUTPUT,
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
        return std::make_shared<DividerXDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void process(const typename TBase::ProcessArgs& args) override;

private:
    using T = float;
    T lastClockValue = 0;
    int counter = 0;
    bool state = false;
    rack::dsp::MinBlepGenerator<16, 16, T> minBlep;

    // debugging
    float timeSinceLastCrossing=0;
    float maxTime=-100;
    float minTime=1000000;
    int numCrossings=0;;
    float totalTime=0;

    OscSmoother2<double> smoother;

 
};


template <class TBase>
inline void DividerX<TBase>::init()
{
}

template <class TBase>
inline void DividerX<TBase>::process(const typename TBase::ProcessArgs& args)
{
    timeSinceLastCrossing += args.sampleTime;
  
    const bool useBlep = TBase::params[MINBLEP_PARAM].value > .5;
    const bool useStabilzer = TBase::params[STABILIZER_PARAM].value > .5f;
    const T inputClockRaw = TBase::inputs[MAIN_INPUT].getVoltage(0);
    const float stabilized = smoother.step(inputClockRaw);
    const float inputToUse = useStabilzer ? stabilized : inputClockRaw;
    
    T deltaClock = inputToUse - lastClockValue;
	T clockCrossing = -lastClockValue / deltaClock;
    lastClockValue = inputToUse;

    float waveForm =  state ? 1.f : -1.f;
    bool newClock =  (0.f < clockCrossing) & (clockCrossing <= 1.f) & (inputToUse >= 0.f);
    if (newClock) {
        float p = clockCrossing - 1.f;
        float x = state ? 2.f : -2.f;

        if (--counter < 0) {
            counter = 0;
        //   counter = 3;
            state = !state;
            waveForm *= -1;
            minBlep.insertDiscontinuity(p, x);
        }       
    }

    float v = waveForm;
    if (useBlep) {
        float blep = minBlep.process(); 
        v -= blep;
    }
    v *= 5;

    TBase::outputs[FIRST_OUTPUT].setVoltage(v, 0);

 //   TBase::outputs[DEBUG_OUTPUT].setVoltage(blep, 0);
    TBase::outputs[DEBUG_OUTPUT].setVoltage(state ? 1.f : -1.f, 0);
    TBase::outputs[STABILIZER_OUTPUT].setVoltage(stabilized, 0);

}

template <class TBase>
int DividerXDescription<TBase>::getNumParams()
{
    return DividerX<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config DividerXDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case DividerX<TBase>::MINBLEP_PARAM:
            ret = {0, 1.0f, 1, "MinBLEP"};
            break;
          case DividerX<TBase>::STABILIZER_PARAM:
            ret = {0, 1.0f, 0, "Stabliize"};
            break;
        default:
            assert(false);
    }
    return ret;
}


