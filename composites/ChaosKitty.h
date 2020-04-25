
#pragma once

#include "AudioMath.h"
#include "Divider.h"
#include "IComposite.h"

#include <assert.h>
#include <algorithm>
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

class Kitty2
{
public:
    float step() {
        double xNext = std::sin(a * y) - std::cos(b * x);
        double yNext = std::sin(c * x) - std::cos(d * y);

        x = xNext;
        y = yNext;

        return x;
    }
private:
    double x = 1;
    double y = 1;

#if 0
    double a = 1.641;
    double b = 1.902;
    double c = .316;
    double d = 1.525;
#endif

#if 1
    double a = -0.827;
    double b = -1.637;
    double c = 1.659;
    double d = -0.943;
#endif
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
    enum class Types {kitty1, kitty2};
    Types type = Types::kitty1;
    Kitty1 kitty1;
    Kitty2 kitty2;
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
    kitty1.setG(g);

    
}

template <class TBase>
inline void ChaosKitty<TBase>::step()
{
    float output = 0;
    if (type == Types::kitty1) {
        output = kitty1.step();
    } else if (type == Types::kitty2) {
        output = kitty2.step();
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
            ret = { -0, 1, 0, "type" };
            break;
        default:
            assert(false);
    }
    return ret;
}


