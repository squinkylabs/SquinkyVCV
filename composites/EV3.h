#pragma once

#include "Divider.h"
#include "IComposite.h"
#ifndef _MSC_VER
#include "MinBLEPVCO.h"
#endif
#include "ObjectCache.h"
#include "SqMath.h"

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack
using Module = ::rack::engine::Module;

template <class TBase>
class EV3Description : public IComposite {
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * perf test 1.0 44.5
 * 44.7 with normalization
 */
template <class TBase>
class EV3 : public TBase {
public:
    friend class TestMB;
    EV3(Module* module) : TBase(module) {
        init();
    }

    EV3() : TBase() {
        init();
    }

    virtual ~EV3() {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription() {
        return std::make_shared<EV3Description<TBase>>();
    }

    enum class Waves {
        SIN,
        TRI,
        SAW,
        SQUARE,
        EVEN,
        NONE,
        END  // just a marker
    };

    enum ParamIds {
        MIX1_PARAM,
        MIX2_PARAM,
        MIX3_PARAM,
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

    enum InputIds {
        CV1_INPUT,
        CV2_INPUT,
        CV3_INPUT,
        FM1_INPUT,
        FM2_INPUT,
        FM3_INPUT,
        PWM1_INPUT,
        PWM2_INPUT,
        PWM3_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        MIX_OUTPUT,
        VCO1_OUTPUT,
        VCO2_OUTPUT,
        VCO3_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    void step() override;

    bool isLoweringVolume() const {
        return volumeScale < 1;
    }

private:
    void setSync();
    void processPitchInputs();
    void processPitchInputs(int osc);
    void processWaveforms();
    void stepVCOs();
    void stepn(int);
    void init();
    void processPWInputs();
    void processPWInput(int osc);
    float getInput(int osc, InputIds in0, InputIds in1, InputIds in2);

    MinBLEPVCO vcos[3];
    float _freq[3];
    float _out[3];
    float _outGain[3];
    float _pitchOffset[3];
    float volumeScale = 1;
    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();

    Divider div;
};

template <class TBase>
inline void EV3<TBase>::init() {
    for (int i = 0; i < 3; ++i) {
        vcos[i].setWaveform(MinBLEPVCO::Waveform::Saw);
        _outGain[i] = 0;
        _pitchOffset[i] = 0;
    }

    div.setup(4, [this] {
        this->stepn(div.getDiv());
    });

    vcos[0].setSyncCallback([this](float f) {
        if (TBase::params[SYNC2_PARAM].value > .5) {
            vcos[1].onMasterSync(f);
        }

        if (TBase::params[SYNC3_PARAM].value > .5) {
            vcos[2].onMasterSync(f);
        }
    });
}

template <class TBase>
inline void EV3<TBase>::setSync() {
    vcos[0].setSyncEnabled(false);
    vcos[1].setSyncEnabled(TBase::params[SYNC2_PARAM].value > .5);
    vcos[2].setSyncEnabled(TBase::params[SYNC3_PARAM].value > .5);
}

template <class TBase>
inline void EV3<TBase>::processWaveforms() {
    vcos[0].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE1_PARAM].value);
    vcos[1].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE2_PARAM].value);
    vcos[2].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE3_PARAM].value);
}

template <class TBase>
float EV3<TBase>::getInput(int osc, InputIds in1, InputIds in2, InputIds in3) {
    const bool in2Connected = TBase::inputs[in2].isConnected();
    const bool in3Connected = TBase::inputs[in3].isConnected();
    InputIds id = in1;
    if ((osc == 1) && in2Connected) {
        id = in2;
    }
    if (osc == 2) {
        if (in3Connected)
            id = in3;
        else if (in2Connected)
            id = in2;
    }
    return TBase::inputs[id].value;
}

template <class TBase>
void EV3<TBase>::processPWInput(int osc) {
    const float pwmInput = getInput(osc, PWM1_INPUT, PWM2_INPUT, PWM3_INPUT) / 5.f;

    const int delta = osc * (OCTAVE2_PARAM - OCTAVE1_PARAM);
    const float pwmTrim = TBase::params[PWM1_PARAM + delta].value;
    const float pwInit = TBase::params[PW1_PARAM + delta].value;

    float pw = pwInit + pwmInput * pwmTrim;
    const float minPw = 0.05f;
    pw = sq::rescale(std::clamp(pw, -1.0f, 1.0f), -1.0f, 1.0f, minPw, 1.0f - minPw);
    vcos[osc].setPulseWidth(pw);
}

template <class TBase>
inline void EV3<TBase>::processPWInputs() {
    processPWInput(0);
    processPWInput(1);
    processPWInput(2);
}

template <class TBase>
inline void EV3<TBase>::stepn(int) {
    // do the mix know taper lookup at a lower sample rate
    for (int i = 0; i < 3; ++i) {
        const float knob = TBase::params[MIX1_PARAM + i].value;
        _outGain[i] = LookupTable<float>::lookup(*audioTaper, knob, false);

        const int delta = i * (OCTAVE2_PARAM - OCTAVE1_PARAM);
        const float finePitch = TBase::params[FINE1_PARAM + delta].value / 12.0f;
        const float semiPitch = TBase::params[SEMI1_PARAM + delta].value / 12.0f;

        float pitch = 1.0f + roundf(TBase::params[OCTAVE1_PARAM + delta].value) +
                      semiPitch +
                      finePitch;
        _pitchOffset[i] = pitch;
    }
    setSync();
    processWaveforms();
    processPWInputs();
}

template <class TBase>
inline void EV3<TBase>::step() {
    div.step();
    processPitchInputs();
    stepVCOs();

    float mix = 0;
    float totalGain = 0;

    for (int i = 0; i < 3; ++i) {
        const float gain = _outGain[i];
        const float rawWaveform = vcos[i].getOutput();
        const float scaledWaveform = rawWaveform * gain;
        totalGain += gain;
        mix += scaledWaveform;
        _out[i] = scaledWaveform;
        TBase::outputs[VCO1_OUTPUT + i].value = rawWaveform;
    }
    if (totalGain <= 1) {
        volumeScale = 1;
    } else {
        volumeScale = 1.0f / totalGain;
    }
    mix *= volumeScale;
    TBase::outputs[MIX_OUTPUT].value = mix;
}

template <class TBase>
inline void EV3<TBase>::stepVCOs() {
    for (int i = 0; i < 3; ++i) {
        vcos[i].step();
    }
}

template <class TBase>
inline void EV3<TBase>::processPitchInputs() {
    float lastFM = 0;
    for (int osc = 0; osc < 3; ++osc) {
        assert(osc >= 0 && osc <= 2);
        const int delta = osc * (OCTAVE2_PARAM - OCTAVE1_PARAM);

        const float cv = getInput(osc, CV1_INPUT, CV2_INPUT, CV3_INPUT);
        float pitch = _pitchOffset[osc];
        pitch += cv;

        float fmCombined = 0;  // The final, scaled, value (post knob
        if (TBase::inputs[FM1_INPUT + osc].isConnected()) {
            const float fm = TBase::inputs[FM1_INPUT + osc].value;
            const float fmDepth = AudioMath::quadraticBipolar(TBase::params[FM1_PARAM + delta].value);
            fmCombined = (fmDepth * fm);
        } else {
            fmCombined = lastFM;
        }
        pitch += fmCombined;
        lastFM = fmCombined;

        const float q = float(log2(261.626));  // move up to pitch range of EvenVCO
        pitch += q;
        const float freq = expLookup(pitch);
        _freq[osc] = freq;
        vcos[osc].setNormalizedFreq(TBase::engineGetSampleTime() * freq,
                                    TBase::engineGetSampleTime());
    }
}

template <class TBase>
int EV3Description<TBase>::getNumParams() {
    return EV3<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config EV3Description<TBase>::getParam(int i) {
    const float numWaves = (float)EV3<TBase>::Waves::END;
    const float defWave = (float)EV3<TBase>::Waves::SIN;

    Config ret(0, 1, 0, "");
    switch (i) {
        case EV3<TBase>::MIX1_PARAM:
            ret = {0.0f, 1.0f, 0, "VCO 1 level"};
            break;
        case EV3<TBase>::MIX2_PARAM:
            ret = {0.0f, 1.0f, 0, "VCO 2 level"};
            break;
        case EV3<TBase>::MIX3_PARAM:
            ret = {0.0f, 1.0f, 0, "VCO 3 level"};
            break;
        case EV3<TBase>::OCTAVE1_PARAM:
            ret = {-5.0f, 4.0f, 0.f, "Octave transpose (VCO 1)"};
            break;
        case EV3<TBase>::SEMI1_PARAM:
            ret = {-11.f, 11.0f, 0.f, "Semitone transpose (VCO 1)"};
            break;
        case EV3<TBase>::FINE1_PARAM:
            ret = {-1.0f, 1.0f, 0, "Fine tune (VCO 1)"};
            break;
        case EV3<TBase>::FM1_PARAM:
            ret = {0.f, 1.f, 0, "Pitch modulation depth (VCO 1)"};
            break;
        case EV3<TBase>::SYNC1_PARAM:
            ret = {0.0f, 1.0f, 0.0f, "unused"};
            ret.active = false;  // this one is unused
            break;
        case EV3<TBase>::WAVE1_PARAM:
            ret = {0.0f, numWaves, defWave, "Waveform (VCO 1)"};
            break;
        case EV3<TBase>::PW1_PARAM:
            ret = {-1.0f, 1.0f, 0, "Pulse width (VCO 1)"};
            break;
        case EV3<TBase>::PWM1_PARAM:
            ret = {-1.0f, 1.0f, 0, "Pulse width modulation (VCO 1)"};
            break;
        case EV3<TBase>::OCTAVE2_PARAM:
            ret = {-5.0f, 4.0f, 0.f, "Octave transpose (VCO 2)"};
            break;
        case EV3<TBase>::SEMI2_PARAM:
            ret = {-11.f, 11.0f, 0.f, "Semitone transpose (VCO 2)"};
            break;
        case EV3<TBase>::FINE2_PARAM:
            ret = {-1.0f, 1.0f, 0, "Fine tune (VCO 2)"};
            break;
        case EV3<TBase>::FM2_PARAM:
            ret = {0.f, 1.f, 0, "Pitch modulation depth (VCO 2)"};
            break;
        case EV3<TBase>::SYNC2_PARAM:
            ret = {0.0f, 1.0f, 0.0f, "Hard sync, VCO 1->2"};
            break;
        case EV3<TBase>::WAVE2_PARAM:
            ret = {0.0f, numWaves, defWave, "Waveform (VCO 2)"};
            break;
        case EV3<TBase>::PW2_PARAM:
            ret = {-1.0f, 1.0f, 0, "Pulse width (VCO 2)"};
            break;
        case EV3<TBase>::PWM2_PARAM:
            ret = {-1.0f, 1.0f, 0, "Pulse width modulation (VCO 2)"};
            break;

        case EV3<TBase>::OCTAVE3_PARAM:
            ret = {-5.0f, 4.0f, 0.f, "Octave transpose (VCO 3)"};
            break;
        case EV3<TBase>::SEMI3_PARAM:
            ret = {-11.f, 11.0f, 0.f, "Semitone transpose (VCO 3)"};
            break;
        case EV3<TBase>::FINE3_PARAM:
            ret = {-1.0f, 1.0f, 0, "Fine tune (VCO 3)"};
            break;
        case EV3<TBase>::FM3_PARAM:
            ret = {0.f, 1.f, 0, "Pitch modulation depth (VCO 3)"};
            break;
        case EV3<TBase>::SYNC3_PARAM:
            ret = {0.0f, 1.0f, 0.0f, "Hard sync, VCO 1->3"};
            break;
        case EV3<TBase>::WAVE3_PARAM:
            ret = {0.0f, numWaves, defWave, "Waveform (VCO 3)"};
            break;
        case EV3<TBase>::PW3_PARAM:
            ret = {-1.0f, 1.0f, 0, "Pulse width (VCO 3)"};
            break;
        case EV3<TBase>::PWM3_PARAM:
            ret = {-1.0f, 1.0f, 0, "Pulse width modulation (VCO 3)"};
            break;
        default:
            assert(false);
    }
    return ret;
}
