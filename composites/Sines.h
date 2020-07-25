
#pragma once

#include "ADSR4.h"
#include "Divider.h"
#include "IComposite.h"
#include "PitchUtils.h"
#include "SinesVCO.h"

#include <assert.h>
#include <memory>

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class SinesDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Sines : public TBase
{
public:
    using T = float_4;

    Sines(Module * module) : TBase(module)
    {
    }
    Sines() : TBase()
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
        V_OCT_INPUT,
        GATE_INPUT,
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
        return std::make_shared<SinesDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:

    SinesVCO<T> sines[3];
    ADSR4 adsr;
    
    Divider divn;

    void stepn();

};


template <class TBase>
inline void Sines<TBase>::init()
{
    divn.setup(4, [this]() {
        this->stepn();
    });
}

template <class TBase>
inline void Sines<TBase>::stepn()
{
    const float semi = PitchUtils::semitone;
    float pitches[12] = {
        -1, 0, 7 * semi, 1,
        1 + 7 * semi, 2, 2 + 4 * semi, 2 + 7 * semi,
        3, 0, 0, 0};
    
    const float cv = Sines<TBase>::inputs[V_OCT_INPUT].getVoltage(0);
    float_4 basePitch(cv);

    float_4 pitch = basePitch;
    pitch += float_4::load(pitches);
    sines[0].setPitch(pitch);

    float* p = pitches + 4;
    pitch = basePitch;
    pitch += float_4::load(p);
    sines[1].setPitch(pitch);

    p = pitches + 8;
    pitch = basePitch;
    pitch += float_4::load(p);
    sines[2].setPitch(pitch);
}

template <class TBase>
inline void Sines<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();


    T sum;
    T deltaT(args.sampleTime);
    sum = sines[0].process(deltaT);
    sum += sines[1].process(deltaT);
 //   sum += sines[2].process(deltaT);
    float s = sum[0] + sum[1] + sum[2] + sum[3];
    s += sines[2].process(deltaT)[0];

    bool gate = Sines<TBase>::inputs[GATE_INPUT].getVoltage(0) > 1;
    if (!gate) {
        s = 0;
    }
    Sines<TBase>::outputs[MAIN_OUTPUT].setVoltage(s, 0);
}

template <class TBase>
int SinesDescription<TBase>::getNumParams()
{
    return Sines<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SinesDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Sines<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


