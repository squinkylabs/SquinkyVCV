#pragma once

#include "IIRUpsampler.h"
#include "IIRDecimator.h"
#include "LookupTable.h"
#include "AsymWaveShaper.h"
#include "ObjectCache.h"

/**
Version 1, cpu usage:
    full wave: 95
    crush: 281
    asy:149
    fold: 102
    fold2: 154

    X4 on input scanning:
    full wave: 85
    crush: 278
    asy:163
    fold: 89
    fold2: 154

    inline:
    fw: 75
    crush: 87
    asy: 112
    fold: 77
    fold2: 136


 */
template <class TBase>
class Shaper : public TBase
{
public:
    Shaper(struct Module * module) : TBase(module)
    {
        init();
    }
    Shaper() : TBase()
    {
        init();
    }

    enum class Shapes
    {
        AsymSpline,
        Clip,
        EmitterCoupled,
        FullWave,
        HalfWave,
        Fold,
        Fold2,
        Crush,
        Invalid
    };

    static const char* getString(Shapes);

    enum ParamIds
    {
        PARAM_SHAPE,
        PARAM_GAIN,
        PARAM_GAIN_TRIM,
        PARAM_OFFSET,
        PARAM_OFFSET_TRIM,
        PARAM_OVERSAMPLE,
        NUM_PARAMS
    };

    enum InputIds
    {
        INPUT_AUDIO,
        INPUT_GAIN,
        INPUT_OFFSET,
        NUM_INPUTS
    };

    enum OutputIds
    {
        OUTPUT_AUDIO,
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

    float _gain = 0;
    float _offset = 0;
    float _gainInput = 0;

private:
    std::shared_ptr<LookupTableParams<float>> audioTaper = {ObjectCache<float>::getAudioTaper()};
    std::shared_ptr<LookupTableParams<float>> sinLookup = {ObjectCache<float>::getSinLookup()};
    AudioMath::ScaleFun<float> scaleGain = AudioMath::makeLinearScaler<float>(0, 1);
    AudioMath::ScaleFun<float> scaleOffset = AudioMath::makeLinearScaler<float>(-5, 5);

    const static int oversample = 16;
    void init();
    IIRUpsampler<oversample> up;
    IIRDecimator<oversample> dec;
    std::shared_ptr<LookupTableParams<float>> tanhLookup;
    AsymWaveShaper asymShaper;
    int cycleCount = 1;
    Shapes shape = Shapes::Clip;
    int asymCurveindex = 0;

    void processCV();
};

template <class TBase>
const char* Shaper<TBase>::getString(Shapes shape)
{
    const char* ret = "";
    switch (shape) {
        case Shapes::Clip:
            ret = "Clip";
            break;
        case Shapes::EmitterCoupled:
            ret = "Emitter Coupled";
            break;
        case Shapes::FullWave:
            ret = "Full Wave";
            break;
        case Shapes::HalfWave:
            ret = "Half Wave";
            break;
        case Shapes::Fold:
            ret = "Folder";
            break;
        case Shapes::Fold2:
            ret = "Folder II";
            break;
        case Shapes::AsymSpline:
            ret = "Asymmetric";
            break;
        case Shapes::Crush:
            ret = "Crush";
            break;
        default:
            assert(false);
            ret = "error";
    }
    return ret;
}


template <class TBase>
void  Shaper<TBase>::init()
{
    float fc = .25 / float(oversample);
    up.setCutoff(fc);
    dec.setCutoff(fc);

    tanhLookup = ObjectCache<float>::getTanh5();
}

template <class TBase>
void Shaper<TBase>::processCV()
{
    // 0..1
    _gainInput = scaleGain(
        TBase::inputs[INPUT_GAIN].value,
        TBase::params[PARAM_GAIN].value,
        TBase::params[PARAM_GAIN_TRIM].value);

    _gain = 5 * LookupTable<float>::lookup(*audioTaper, _gainInput, false);

    // -5 .. 5
    const float offsetInput = scaleOffset(
        TBase::inputs[INPUT_OFFSET].value,
        TBase::params[PARAM_OFFSET].value,
        TBase::params[PARAM_OFFSET_TRIM].value);

    _offset = offsetInput;

    const int iShape = (int) std::round(TBase::params[PARAM_SHAPE].value);
    shape = Shapes(iShape);

    const float sym = .1f * (5 - _offset);
    asymCurveindex = (int) round(sym * 15.1);           // This math belongs in the shaper
}


#if 1
template <class TBase>
void  Shaper<TBase>::step()
{
    if (--cycleCount < 0) {
        cycleCount = 7;
        processCV();
    }

    float buffer[oversample];
    float input = TBase::inputs[INPUT_AUDIO].value;
   // const float rawInput = input;

    // TODO: maybe add offset after gain?
    if (shape != Shapes::AsymSpline) {
        input += _offset;
    }
    if (shape != Shapes::Crush) {
        input *= _gain;
    }

    up.process(buffer, input);

    switch (shape) {
        case Shapes::FullWave:
            for (int i = 0; i < oversample; ++i) {
                float x = buffer[i];
                x = std::abs(x);
                x = std::min(x, 10.f);
                buffer[i] = x;
            }
            break;
        case  Shapes::AsymSpline:
            for (int i = 0; i < oversample; ++i) {
                float x = buffer[i];
                x *= .15f;
                x = asymShaper.lookup(x, asymCurveindex);
                x *= 6.1f;
                buffer[i] = x;
            }
            break;
        case Shapes::Clip:
            for (int i = 0; i < oversample; ++i) {
                float x = buffer[i];
                x *= 3;
                x = std::min(3.f, x);
                x = std::max(-3.f, x);
                x *= 1.2f;
                buffer[i] = x;
            }
            break;
        case Shapes::EmitterCoupled:
            for (int i = 0; i < oversample; ++i) {
                float x = buffer[i];
                x *= .25;
                x = LookupTable<float>::lookup(*tanhLookup.get(), x, true);
                x *= 5.4f;
                buffer[i] = x;
            }
            break;

        case Shapes::HalfWave:
            for (int i = 0; i < oversample; ++i) {
                float x = buffer[i];
                x = std::max(0.f, x);
                x *= 1.4f;
                x = std::min(x, 10.f);
                buffer[i] = x;
            }
            break;
        case Shapes::Fold:
            for (int i = 0; i < oversample; ++i) {
                float x = buffer[i];
                x = AudioMath::fold(x);
                x *= 5.6f;
                buffer[i] = x;
            }
            break;
        case Shapes::Fold2:
            for (int i = 0; i < oversample; ++i) {
                float x = buffer[i];
                x = .3f * AudioMath::fold(x);
                if (x > 0) {
                    x = LookupTable<float>::lookup(*sinLookup, 1.3f * x, false);
                } else {
                    x = -LookupTable<float>::lookup(*sinLookup, -x, false);
                }
                if (x > 0) x = std::sqrt(x);
                x *= 4.4f;
                buffer[i] = x;
            }
            break;
        case Shapes::Crush:
        {
            float invGain = 1 + (1 - _gainInput) * 100; //0..10
            invGain *= .01f;
            for (int i = 0; i < oversample; ++i) {
                float x = buffer[i];            // for crush, no gain has been applied

#if 0
                if (invGain < 1) {
                    printf("invg gain = %f\n", invGain);
                    fflush(stdout);
                    invGain = 1;

                }
#endif
            //  printf("crush, x=%.2f, gi=%.2f invGain = %.2f", x, gainInput, invGain);

                x *= invGain;
                x = std::round(x);
                //  printf(" mult=%.2f", x);
                x /= invGain;
                //   printf(" dv=%.2f\n", x);    fflush(stdout);
                buffer[i] = x;
            }
        }
        break;


        default:
            assert(false);
    }


/*
    else

        for (int i = 0; i < oversample; ++i) {
        float x = buffer[i];



            case Shapes::Fold:
                x = AudioMath::fold(x);
                x *= 5.6f;
                break;
            case Shapes::Fold2:
                x = .3f * AudioMath::fold(x);
                if (x > 0) {
                    x = LookupTable<float>::lookup(*sinLookup, 1.3f * x, false);
                } else {
                    x = -LookupTable<float>::lookup(*sinLookup, -x, false);
                }
                if (x > 0) x = std::sqrt(x);
                x *= 4.4f;

                break;
            case Shapes::AsymSpline:
            {
                x = rawInput * _gain;           // we use the offset for something else
                x *= .15f;
                const float sym = .1f * (5 - _offset);
                int index = (int) round(sym * 15.1);           // This math belongs in the shaper
                x = asymShaper.lookup(x, index);
                x *= 6.1f;
            }
            break;
            case Shapes::Crush:
            {
                x = rawInput;          // remove the gain
                float invGain = 1 + (1 - _gainInput) * 100; //0..10
                invGain *= .01f;
#if 0
                if (invGain < 1) {
                    printf("invg gain = %f\n", invGain);
                    fflush(stdout);
                    invGain = 1;

                }
#endif
                //  printf("crush, x=%.2f, gi=%.2f invGain = %.2f", x, gainInput, invGain);

                x *= invGain;
                x = std::round(x);
                //  printf(" mult=%.2f", x);
                x /= invGain;
                //   printf(" dv=%.2f\n", x);
                fflush(stdout);
            }
            break;
            default:
                assert(false);

        }
        buffer[i] = x;
    }
#endif
*/

    const float output = dec.process(buffer);
    TBase::outputs[OUTPUT_AUDIO].value = output;
}
#else
template <class TBase>
void  Shaper<TBase>::step()
{
    float buffer[oversample];
    float input = TBase::inputs[INPUT_AUDIO].value;

    up.process(buffer, input);

    const float output = dec.process(buffer);
    TBase::outputs[OUTPUT_AUDIO].value = output;
}
#endif
