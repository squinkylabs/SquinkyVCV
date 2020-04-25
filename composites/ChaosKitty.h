
#pragma once

#include "AudioMath.h"
#include "Divider.h"
#include "IComposite.h"

#include <assert.h>
#include <memory>

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

class Kitty1
{
public:
    float step() {
        const float next = g * x * (1 - x);
        x = next;
      //  printf("%f\n", x);fflush(stdout);
        return float(5 * (next - .5));
    }
    void setG(float _g) {
        if (_g >= 4) {
            g = 3.99999f;
        }
        g = _g;
    }

    float _getG() const {
        return g;
    }
private:
    float x = .5f;
    float g = 3.9f; 
};

template <class TBase>
class ChaosKitty : public TBase
{
public:

    ChaosKitty(Module * module) : TBase(module)
    {
        printf("chaos ctr 1\n"); fflush(stdout);
    }
    ChaosKitty() : TBase()
    {
        printf("chaos ctr 2\n"); fflush(stdout);
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
        return kitty1._getG();
    }

private:
    Kitty1 kitty1;
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
    const float chaosCV = TBase::inputs[CHAOS_INPUT].getVoltage(0) / 10.f;
    const float g  = scaleChaos(
        chaosCV,
        TBase::params[CHAOS_PARAM].value,
        TBase::params[CHAOS_TRIM_PARAM].value);
  //  printf("g = %.2f\n", g); fflush(stdout);
    kitty1.setG(g);
}

template <class TBase>
inline void ChaosKitty<TBase>::step()
{
    const float f = kitty1.step();
    TBase::outputs[MAIN_OUTPUT].setVoltage(f, 0);

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
            ret = { -0, 1, 0, "type" };
            break;
        default:
            assert(false);
    }
    return ret;
}


