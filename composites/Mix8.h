
#pragma once

#include <assert.h>
#include <memory>
#include "IComposite.h"

template <class TBase>
class Mix8Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Mix8 : public TBase
{
public:

    Mix8(struct Module * module) : TBase(module)
    {
    }
    Mix8() : TBase()
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
        NUM_INPUTS
    };

    enum OutputIds
    {
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
        return std::make_shared<Mix8Description<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:

};


template <class TBase>
inline void Mix8<TBase>::init()
{
}


template <class TBase>
inline void Mix8<TBase>::step()
{
}

template <class TBase>
int Mix8Description<TBase>::getNumParams()
{
    return Mix8<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config Mix8Description<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Mix8<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


