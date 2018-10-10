
#pragma once


template <class TBase>
class Super : public TBase
{
public:

    Super(struct Module * module) : TBase(module)
    {
    }
    Super() : TBase()
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

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:

};


template <class TBase>
inline void Super<TBase>::init()
{
}


template <class TBase>
inline void Super<TBase>::step()
{
}

