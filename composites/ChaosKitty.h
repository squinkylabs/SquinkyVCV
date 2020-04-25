
#pragma once

#include "AudioMath.h"
#include "Chaos.h"
#include "Divider.h"
#include "FractionalDelay.h"
#include "IComposite.h"

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
        return { "noise", "pitched" };
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
        TYPE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CHAOS_INPUT,
        CHAOS2_INPUT,
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

    float _getG() const {
        return simpleChaoticNoise._getG();
    }

private:
    enum class Types { SimpleChaoticNoise, ResonantNoise};
    Types type = Types::SimpleChaoticNoise;
    SimpleChaoticNoise simpleChaoticNoise;
    ResonantNoise resonantNoise;
  //  Kitty2 kitty2;
 //   Kitty3 kitty3;
  //  Kitty4 kitty4;
    AudioMath::ScaleFun<float> scaleChaos;

    Divider div;
    void stepn(int);
};


template <class TBase>
inline void ChaosKitty<TBase>::init()
{
    div.setup(4, [this] {
        this->stepn(div.getDiv());
     });

    scaleChaos = AudioMath::makeLinearScaler<float>(3.5, 4);
}

template <class TBase>
inline void ChaosKitty<TBase>::stepn(int n) {
    type = Types(int(std::round(TBase::params[TYPE_PARAM].value)));
   // printf("type = %d, value = %.2f\n", type, TBase::params[TYPE_PARAM].value);
   //  fflush(stdout);;

    const float chaosCV = TBase::inputs[CHAOS_INPUT].getVoltage(0) / 10.f;
    const float g  = scaleChaos(
        chaosCV,
        TBase::params[CHAOS_PARAM].value,
        TBase::params[CHAOS_TRIM_PARAM].value);
  //  printf("g = %.2f\n", g); fflush(stdout);
    simpleChaoticNoise.setG(g);
    resonantNoise.setG(g);
#if 0

    float k2 = TBase::params[CHAOS_PARAM].value * .001;
    kitty2.setDelta(k2);
    kitty4.setDelta(k2);
#endif
}

template <class TBase>
inline void ChaosKitty<TBase>::step()
{
    float output = 0;
    if (type == Types::SimpleChaoticNoise) {
        output = simpleChaoticNoise.step();
    } else if (type == Types::ResonantNoise) {
        output = resonantNoise.step();
    } else {
       // assert(false);
    }

    output = std::min(output, 5.f);
    output = std::max(output, -5.f);
    TBase::outputs[MAIN_OUTPUT].setVoltage(output, 0);

    div.step();
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
            ret = { 0, 3, 0, "type" };
            break;
        default:
            assert(false);
    }
    return ret;
}


