#pragma once

#include "MinBLEPVCO.h"
#include "ObjectCache.h"

/**
 *
 */
template <class TBase>
class EV3 : public TBase
{
public:
    friend class TestMB;
    EV3(struct Module * module) : TBase(module)
    {
        init();
    }
    
    EV3() : TBase()
    {
        init();
    }

    enum class Waves
    {
        SIN,
        TRI,
        SAW,
        SQUARE,
        EVEN,
        NONE,
        END     // just a marker
    };
    
    enum ParamIds
    {
        OCTAVE1_PARAM,
        SEMI1_PARAM,
        FINE1_PARAM,
        FM1_PARAM,
        SYNC1_PARAM,
        WAVE1_PARAM,
        PW1_PARAM,
        PWM1_PARAM,

        OCTAVE2_PARAM,
        SEMI2_PARAM,
        FINE2_PARAM,
        FM2_PARAM,
        SYNC2_PARAM,
        WAVE2_PARAM,
        PW2_PARAM,
        PWM2_PARAM,

        OCTAVE3_PARAM,
        SEMI3_PARAM,
        FINE3_PARAM,
        FM3_PARAM,
        SYNC3_PARAM,
        WAVE3_PARAM,
        PW3_PARAM,
        PWM3_PARAM,

        NUM_PARAMS
    };
 
    enum InputIds
    {
        CV1_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MIX_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    void step() override;
 
private:
    void processPitchInputs();
    void processPitchInputs(int osc);
    void processWaveforms();
    void stepVCOs();
    void init();

    MinBLEPVCO vcos[3];
    float _freq[3];
    float _out[3];
    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
};

template <class TBase>
inline void EV3<TBase>::init()
{
    for (int i = 0; i < 3; ++i) {
#if 0
        vcos[i].enableWaveform(MinBLEPVCO::Waveform::Saw, true);
        vcos[i].enableWaveform(MinBLEPVCO::Waveform::Tri, false);
        vcos[i].enableWaveform(MinBLEPVCO::Waveform::Sin, false);
        vcos[i].enableWaveform(MinBLEPVCO::Waveform::Square, false);
        vcos[i].enableWaveform(MinBLEPVCO::Waveform::Even, false);
#endif
        vcos[i].setWaveform(MinBLEPVCO::Waveform::Saw);
    }

    vcos[0].setSyncCallback([this](float f, float dx) {
    
        if (TBase::params[SYNC2_PARAM].value > .5) {
            vcos[1].onMasterSync(f, dx);
        }
        if (TBase::params[SYNC3_PARAM].value > .5) {
            vcos[2].onMasterSync(f, dx);
        }
     });
}

template <class TBase>
inline void EV3<TBase>::processWaveforms()
{
    vcos[0].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE1_PARAM].value);
    vcos[1].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE2_PARAM].value);
    vcos[2].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE3_PARAM].value);
}

template <class TBase>
inline void EV3<TBase>::step()
{
    processPitchInputs();
    processWaveforms();
    stepVCOs();
    float mix = 0;

    for (int i = 0; i < 3; ++i) {
        const float wf = vcos[i].getOutput();
#if 0
        if (i == 0 && TBase::params[WAVE1_PARAM].value == (float) Waves::SAW) {
            mix += wf;
        }
        if (i == 1 && TBase::params[WAVE2_PARAM].value == (float)Waves::SAW) {
            mix += wf;
        }
        if (i == 2 && TBase::params[WAVE3_PARAM].value == (float)Waves::SAW) {
            mix += wf;
        }
#endif
        mix += wf;
        _out[i] = wf;
    }

    TBase::outputs[MIX_OUTPUT].value = mix;
}

template <class TBase>
inline void EV3<TBase>::stepVCOs()
{
    for (int i = 0; i < 3; ++i) {
        vcos[i].step();
    }
    // route waveform outputs?
}

template <class TBase>
inline void EV3<TBase>::processPitchInputs()
{
    processPitchInputs(0);
    processPitchInputs(1);
    processPitchInputs(2);

    static float last = 0;
    auto p = TBase::params[WAVE1_PARAM].value;
    if (p != last) {
        printf("wavform param = %f\n", TBase::params[WAVE1_PARAM].value);
        last = p;
    }
}

template <class TBase>
inline void EV3<TBase>::processPitchInputs(int osc)
{
    assert(osc >= 0 && osc <= 2);
    const int delta = osc * (OCTAVE2_PARAM - OCTAVE1_PARAM);

    float pitch = 1.0f + roundf(TBase::params[OCTAVE1_PARAM + delta].value) +
        TBase::params[SEMI1_PARAM + delta].value / 12.0f +
        TBase::params[FINE1_PARAM + delta].value / 12.0f;
    pitch += TBase::inputs[CV1_INPUT].value;
#if 0
    pitch += TBase::inputs[CV_INPUT].value;
    pitch += .25f * TBase::inputs[PITCH_MOD_INPUT].value *
        taper(TBase::params[PARAM_PITCH_MOD_TRIM].value);
#endif

    const float q = float(log2(261.626));       // move up to pitch range of even vco
    pitch += q;
    const float freq = expLookup(pitch);
    _freq[osc] = freq;
    vcos[osc].setNormalizedFreq(TBase::engineGetSampleTime() * freq, TBase::engineGetSampleTime());
}



