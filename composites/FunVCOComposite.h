#pragma once


#include "FunVCO.h"

#include "IComposite.h"

//#define _ORIGVCO

template <class TBase>
class FunVCOComposite : public TBase, public IComposite
{
public:
    FunVCOComposite()
    {
        init();
    }
    FunVCOComposite(struct Module * module) : TBase(module)
    {
        init();
    }
    enum ParamIds
    {
        MODE_PARAM,
        SYNC_PARAM,
        FREQ_PARAM,
        FINE_PARAM,
        FM_PARAM,
        PW_PARAM,
        PWM_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        PITCH_INPUT,
        FM_INPUT,
        SYNC_INPUT,
        PW_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        SIN_OUTPUT,
        TRI_OUTPUT,
        SAW_OUTPUT,
        SQR_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
     Config getParam(int i) override;
     int getNumParams() override
     {
         return NUM_PARAMS;
     }


    void step() override;
    void init()
    {
        oscillator.init();
    }

    void setSampleRate(float rate)
    {
        oscillator.sampleTime = 1.f / rate;
    }

private:
#ifdef _ORIGVCO
    VoltageControlledOscillatorOrig<16, 16> oscillator;
#else
    VoltageControlledOscillator<16, 16> oscillator;
#endif
};

template <class TBase>
inline typename FunVCOComposite<TBase>::Config
    FunVCOComposite<TBase>::getParam(int i)
{
    Config ret(0, 1, 0);
    switch(i) {
        case MODE_PARAM:
            ret = {0.0f, 1.0f, 1.0f};
            break;
        case SYNC_PARAM:
            ret = {0.0f, 1.0f, 1.0f};
            break;
        case FREQ_PARAM:
            ret = {-54.0f, 54.0f, 0.0f};
            break;
        case FINE_PARAM:
            ret = {-1.0f, 1.0f, 0.0f};
            break;
        case FM_PARAM:
            ret = {0.0f, 1.0f, 0.0f};
            break;
        case PW_PARAM:
            ret = {0.0f, 1.0f, 0.5f};
            break;
        case PWM_PARAM:
            ret = {0.0f, 1.0f, 0.0f};
            break;
        default:
            assert(false);
    }
    return ret;
}

template <class TBase>
inline void FunVCOComposite<TBase>::step()
{
    oscillator.analog = TBase::params[MODE_PARAM].value > 0.0f;
    oscillator.soft = TBase::params[SYNC_PARAM].value <= 0.0f;

    float pitchFine = 3.0f * rack::quadraticBipolar(TBase::params[FINE_PARAM].value);
    float pitchCv = 12.0f * TBase::inputs[PITCH_INPUT].value;
    if (TBase::inputs[FM_INPUT].active) {
        pitchCv += rack::quadraticBipolar(TBase::params[FM_PARAM].value) * 12.0f * TBase::inputs[FM_INPUT].value;
    }

    oscillator.setPitch(TBase::params[FREQ_PARAM].value, pitchFine + pitchCv);


    oscillator.setPulseWidth(TBase::params[PW_PARAM].value + TBase::params[PWM_PARAM].value * TBase::inputs[PW_INPUT].value / 10.0f);
    oscillator.syncEnabled = TBase::inputs[SYNC_INPUT].active;

#ifndef _ORIGVCO
    oscillator.sawEnabled = TBase::outputs[SAW_OUTPUT].active;
    oscillator.sinEnabled = TBase::outputs[SIN_OUTPUT].active;
    oscillator.sqEnabled = TBase::outputs[SQR_OUTPUT].active;
    oscillator.triEnabled = TBase::outputs[TRI_OUTPUT].active;
#endif

    oscillator.process(TBase::engineGetSampleTime(), TBase::inputs[SYNC_INPUT].value);
    // Set output
    if (TBase::outputs[SIN_OUTPUT].active)
        TBase::outputs[SIN_OUTPUT].value = 5.0f * oscillator.sin();
    if (TBase::outputs[TRI_OUTPUT].active)
        TBase::outputs[TRI_OUTPUT].value = 5.0f * oscillator.tri();
    if (TBase::outputs[SAW_OUTPUT].active)
        TBase::outputs[SAW_OUTPUT].value = 5.0f * oscillator.saw();
    if (TBase::outputs[SQR_OUTPUT].active)
        TBase::outputs[SQR_OUTPUT].value = 5.0f * oscillator.sqr();

}