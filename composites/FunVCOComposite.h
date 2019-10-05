#pragma once

#include "FunVCO.h"
#include "IComposite.h"
#include "SqPort.h"

template <class TBase>
class FunDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class FunVCOComposite : public TBase
{
public:
    FunVCOComposite()
    {
        init();
    }
    FunVCOComposite(Module * module) : TBase(module)
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
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<FunDescription<TBase>>();
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
int FunDescription<TBase>::getNumParams()
{
    return FunVCOComposite<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config FunDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch(i) {
        case FunVCOComposite<TBase>::MODE_PARAM:
            ret = {0.0f, 1.0f, 1.0f, "Analog/digital mode"};
            break;
        case FunVCOComposite<TBase>::SYNC_PARAM:
            ret = {0.0f, 1.0f, 1.0f, "Sync hard/soft"};
            break;
        case FunVCOComposite<TBase>::FREQ_PARAM:
            ret = {-54.0f, 54.0f, 0.0f, "Frequency"};
            break;
        case FunVCOComposite<TBase>::FINE_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Fine frequency"};
            break;
        case FunVCOComposite<TBase>::FM_PARAM:
            ret = {0.0f, 1.0f, 0.0f, "Pitch modulation depth"};
            break;
        case FunVCOComposite<TBase>::PW_PARAM:
            ret = {0.0f, 1.0f, 0.5f, "Pulse width"};
            break;
        case FunVCOComposite<TBase>::PWM_PARAM:
            ret = {0.0f, 1.0f, 0.0f, "Pulse width modulation depth"};
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

    float pitchFine = 3.0f * sq::quadraticBipolar(TBase::params[FINE_PARAM].value);
    float pitchCv = 12.0f * TBase::inputs[PITCH_INPUT].getVoltage(0);
    if (SqPort::isConnected(TBase::inputs[FM_INPUT])) {
        pitchCv += sq::quadraticBipolar(TBase::params[FM_PARAM].value) * 12.0f * TBase::inputs[FM_INPUT].getVoltage(0);
    }

    oscillator.setPitch(TBase::params[FREQ_PARAM].value, pitchFine + pitchCv);


    oscillator.setPulseWidth(TBase::params[PW_PARAM].value + TBase::params[PWM_PARAM].value * TBase::inputs[PW_INPUT].value / 10.0f);
    oscillator.syncEnabled = SqPort::isConnected(TBase::inputs[SYNC_INPUT]);

#ifndef _ORIGVCO
    oscillator.sawEnabled = SqPort::isConnected(TBase::outputs[SAW_OUTPUT]);
    oscillator.sinEnabled = SqPort::isConnected(TBase::outputs[SIN_OUTPUT]);
    oscillator.sqEnabled = SqPort::isConnected(TBase::outputs[SQR_OUTPUT]);
    oscillator.triEnabled = SqPort::isConnected(TBase::outputs[TRI_OUTPUT]);
#endif

    oscillator.process(TBase::engineGetSampleTime(), TBase::inputs[SYNC_INPUT].value);
    // Set output
    if (SqPort::isConnected(TBase::outputs[SIN_OUTPUT]))
        TBase::outputs[SIN_OUTPUT].value = 5.0f * oscillator.sin();
    if (SqPort::isConnected(TBase::outputs[TRI_OUTPUT]))
        TBase::outputs[TRI_OUTPUT].value = 5.0f * oscillator.tri();
    if (SqPort::isConnected(TBase::outputs[SAW_OUTPUT]))
        TBase::outputs[SAW_OUTPUT].value = 5.0f * oscillator.saw();
    if (SqPort::isConnected(TBase::outputs[SQR_OUTPUT]))
        TBase::outputs[SQR_OUTPUT].value = 5.0f * oscillator.sqr();

}