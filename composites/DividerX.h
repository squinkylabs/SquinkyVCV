
#pragma once

#include <assert.h>
#include <memory>
#include "IComposite.h"

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
        TEST_PARAM,
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
    dsp::MinBlepGenerator<16, 16, T> minBlep;
  //  HalfBlep minBlep;

    // debugging
    float timeSinceLastCrossing=0;
    float maxTime=-100;
    float minTime=1000000;
    int numCrossings=0;;
    float totalTime=0;

 
};


template <class TBase>
inline void DividerX<TBase>::init()
{
}

template <class TBase>
inline void DividerX<TBase>::process(const typename TBase::ProcessArgs& args)
{
    timeSinceLastCrossing += args.sampleTime;
  

    const T inputClock = TBase::inputs[MAIN_INPUT].getVoltage(0);

    const T orig = lastClockValue;

    T deltaClock = inputClock - lastClockValue;
	T clockCrossing = -lastClockValue / deltaClock;
 //  clockCrossing *= .5;
    lastClockValue = inputClock;

    float waveForm =  state ? 1 : -1;
    bool newClock =  (0.f < clockCrossing) & (clockCrossing <= 1.f) & (inputClock >= 0.f);
    if (newClock) {
       //   bool doBlep = false;
        float p = clockCrossing - 1.f;
      //  p *= 2;
        float x = state ? 2 : -2;
    //    waveForm = x * .5;          // capture the value before we flip
 //   printf("X: p=%f x = %f in=%f last=%f clockCrossing=%f\n", p, x, inputClock, orig, clockCrossing);
        
        
        //minBlep.insertDiscontinuity(p, x);
        if (--counter < 0) {
            counter = 0;
        //   counter = 3;
            state = !state;
            waveForm *= -1;
          //  doBlep = true;
            minBlep.insertDiscontinuity(p, x);
        }
#if 0
       
     //    printf("time since last = %f x1000:%f\n", timeSinceLastCrossing, 1000 * timeSinceLastCrossing);
        maxTime = std::max(maxTime, timeSinceLastCrossing);
        minTime = std::min(maxTime, timeSinceLastCrossing);
        ++numCrossings;
        totalTime += timeSinceLastCrossing;
        timeSinceLastCrossing = 0;

       
      //  if (true) {
     //       printf("nc100=%d\n", )
        if (((numCrossings %500) == 0) && (numCrossings != 0)) {
            printf("%d crossings, max=%f min=%f, avg=%f\n", numCrossings, 1000*maxTime, 1000*minTime, 1000*totalTime / numCrossings);
            printf("min/max diff in samples = %f\n", (maxTime - minTime) / args.sampleTime);
            fflush(stdout);
        }
#endif
       
    }

  // waveForm =  state ? 1 : -1;

  //  float v = state ?  1 : -1;
    float v = waveForm;
    float blep = minBlep.process(); 
    v -= blep;
 

    TBase::outputs[FIRST_OUTPUT].setVoltage(v, 0);

    TBase::outputs[DEBUG_OUTPUT].setVoltage(blep, 0);

#if 0
    if (inputPcValue / deltaSync;
			rocessing.trigger()) {
        if (--counter < 0) {
            counter = 0;
            state = !state;
            float v = state ? 1 : -1;
            v *= 5;
            TBase::outputs[FIRST_OUTPUT].setVoltage(v, 0);
        }

    }
#endif
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
        case DividerX<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


