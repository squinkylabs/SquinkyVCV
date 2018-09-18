#pragma once

#include "IIRUpsampler.h"
#include "IIRDecimator.h"
#include "LookupTable.h"
#include "AsymWaveShaper.h"
#include "ObjectCache.h"

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

    enum class Shapes {
        AsymSpline,
        Clip,
        EmitterCoupled,
        FullWave,
        HalfWave,
        Fold,
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
        PARAM_SYMMETRY,
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

private:

    const static int oversample = 16;
    void init();
    IIRUpsampler<oversample> up;
    IIRDecimator<oversample> dec;
    std::shared_ptr<LookupTableParams<float>> tanhLookup;
    AsymWaveShaper asymShaper;
};

template <class TBase>
const char* Shaper<TBase>::getString(Shapes shape)
{
    const char* ret = "";
    switch (shape)
    {
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
        case Shapes::AsymSpline:
            ret = "Asymetric";
            break;
        default:
            assert(false);
    }
    return ret;
}


template <class TBase>
void  Shaper<TBase>::init()
{
   // clock.setMultiplier(1); // no mult
   float fc = .25 / float(oversample);
   up.setCutoff(fc);
   dec.setCutoff(fc);

    tanhLookup = ObjectCache<float>::getTanh5();
}

template <class TBase>
void  Shaper<TBase>::step()
{
    float buffer[oversample];
    float input = TBase::inputs[INPUT_AUDIO].value;
    input += TBase::params[PARAM_OFFSET].value;
    input *= TBase::params[PARAM_GAIN].value;

    const int iShape = (int) std::round(TBase::params[PARAM_SHAPE].value);
    const Shapes shape = Shapes(iShape);

    up.process(buffer, input);
    for (int i=0; i<oversample; ++i) {
        float x = buffer[i];
      
        switch (shape)
        {
            /*
              AsymSpline,
        Clip,
        EmitterCoupled,
        FullWave,
        HalfWave,
        Fold*/
            case Shapes::Clip:   
                x = std::min(1.f, x);
                x = std::max(-1.f, x);
                break;
            case Shapes::EmitterCoupled:
                x = LookupTable<float>::lookup(*tanhLookup.get(), x);
                break;
            case Shapes::FullWave:
                x = std::abs(x);
                break;
            case Shapes::HalfWave:
                x = std::max(0.f, x);
                break;
            case Shapes::Fold:
                x = AudioMath::fold(x);
                break;
            case Shapes::AsymSpline:
                {
                const float sym = TBase::params[PARAM_SYMMETRY].value;    // 0..1
                int index = (int) round(sym * 15.1);           // This match belongs in the shaper
                x = asymShaper.lookup(x, index);
                }
                break;
            default:
                assert(false);

        }
        buffer[i] = x;
    }

    const float output = dec.process(buffer);
    TBase::outputs[OUTPUT_AUDIO].value = output;
} 
