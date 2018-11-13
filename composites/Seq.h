#pragma once

#include "GateTrigger.h"

template <class TBase>
class Seq : public TBase
{
public:
    Seq(struct Module * module) : TBase(module),  gateTrigger(true)
    {
        init();
    }
    Seq() : TBase(),  gateTrigger(true)
    {
        init();
    }

    enum ParamIds
    {

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
private:
    GateTrigger gateTrigger; 
    void init();
};

template <class TBase>
void  Seq<TBase>::init()
{
}



