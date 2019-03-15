
#pragma once

#include <assert.h>
#include <memory>
#include "IComposite.h"

template <class TBase>
class Slew4Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Slew4 : public TBase
{
public:

    Slew4(struct Module * module) : TBase(module)
    {
    }
    Slew4() : TBase()
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
        RISE_PARAM,
        FALL_PARAM,
        LEVEL_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        INPUT_TRIGGER0,
        INPUT_TRIGGER1,
        INPUT_TRIGGER2,
        INPUT_TRIGGER3,
        INPUT_TRIGGER4,
        INPUT_TRIGGER5,
        INPUT_TRIGGER6,
        INPUT_TRIGGER7,
        INPUT_AUDIO0,
        INPUT_AUDIO1,
        INPUT_AUDIO2,
        INPUT_AUDIO3,
        INPUT_AUDIO4,
        INPUT_AUDIO5,
        INPUT_AUDIO6,
        INPUT_AUDIO7,
        NUM_INPUTS
    };

    enum OutputIds
    {
        OUTPUT0,
        OUTPUT1,
        OUTPUT2,
        OUTPUT3,
        OUTPUT4,
        OUTPUT5,
        OUTPUT6,
        OUTPUT7,
        OUTPUT_MIX,
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
        return std::make_shared<Slew4Description<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:

};


template <class TBase>
inline void Slew4<TBase>::init()
{
}


template <class TBase>
inline void Slew4<TBase>::step()
{
}

template <class TBase>
int Slew4Description<TBase>::getNumParams()
{
    return Slew4<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config Slew4Description<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Slew4<TBase>::RISE_PARAM:
            ret = {-5.0f, 5.0f, 0, "Rise time"};
            break;
        case Slew4<TBase>::FALL_PARAM:
            ret = {-5.0f, 5.0f, 0, "Fall time"};
            break;
        case Slew4<TBase>::LEVEL_PARAM:
            ret = {-5.0f, 5.0f, 0, "Level"};
            break;
        default:
            assert(false);
    }
    return ret;
}


