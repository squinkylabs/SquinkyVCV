
#pragma once


#include "Divider.h"
#include "IComposite.h"
#include "LadderFilter.h"
#include "LookupTable.h"
#include "ObjectCache.h"

#include <assert.h>
#include <memory>

#ifdef __V1
namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = rack::engine::Module;
#else
namespace rack {
    struct Module;
};
using Module = rack::Module;
#endif

template <class TBase>
class FiltDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * CPU usage, tanh and no oversampling: 30
 * with 4X: 140
 */
template <class TBase>
class Filt : public TBase
{
public:
    using T = float;
    Filt(Module * module) : TBase(module)
    {
    }
    Filt() : TBase()
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
        FC_PARAM,
        Q_PARAM,
        TYPE_PARAM,
        DRIVE_PARAM,
        VOICING_PARAM,
        STAGING_PARAM,      // aka "edge"
        SPREAD_PARAM,
        POLES_PARAM,
        BASS_MAKEUP_PARAM,
        BASS_MAKEUP_TYPE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        L_AUDIO_INPUT,
        R_AUDIO_INPUT,
        CV_INPUT,
        Q_INPUT,
        DRIVE_INPUT,
        POLES_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        L_AUDIO_OUTPUT,
        R_AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    static std::vector<std::string> getTypeNames()
    {
        return LadderFilter<T>::getTypeNames();
    }

    static std::vector<std::string> getVoicingNames()
    {
        return LadderFilter<T>::getVoicingNames();
    }

    static std::vector<std::string> getBassMakeupNames()
    {
        return LadderFilter<T>::getBassMakeupNames();
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<FiltDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:
    class DSPImp
    {
    public:
        LadderFilter<T> _f;
        bool isActive = false;
    };
    DSPImp dsp[2];
    Divider div;
    std::shared_ptr<LookupTableParams<T>> expLookup = ObjectCache<T>::getExp2();            // Do we need more precision?
    AudioMath::ScaleFun<float> scaleGain = AudioMath::makeLinearScaler<float>(0, 1);
    std::shared_ptr<LookupTableParams<float>> audioTaper = {ObjectCache<float>::getAudioTaper()};

    void stepn(int);
};


template <class TBase>
inline void Filt<TBase>::init()
{
    div.setup(4, [this] {
        this->stepn(div.getDiv());
        });
}

template <class TBase>
inline void Filt<TBase>::stepn(int)
{

    // get param -5..5
    T x = TBase::params[FC_PARAM].value;

    // now 0..10
    x += 5;
    x += TBase::inputs[CV_INPUT].value;

    const T fc = LookupTable<T>::lookup(*expLookup, x, true) * 10;
    {
        static float c = -1;
        if (c != fc) {
            c = fc;
            printf("setting Fc to %f\n", fc); fflush(stdout);
        }
    }
    const T normFc = fc * TBase::engineGetSampleTime();
    T fcClipped = std::min(normFc, T(.48));
    fcClipped = std::max(normFc, T(.0000001));
    
    const float res = TBase::params[Q_PARAM].value;
    const LadderFilter<T>::Types type = (LadderFilter<T>::Types) (int) std::round(TBase::params[TYPE_PARAM].value);
    const LadderFilter<T>::Voicing voicing = (LadderFilter<T>::Voicing) (int) std::round(TBase::params[VOICING_PARAM].value);
   
    //********* now the drive 
        // 0..1
    float  gainInput = scaleGain(
     //   TBase::inputs[INPUT_GAIN].value,
        0,
        TBase::params[DRIVE_PARAM].value,
       // TBase::params[PARAM_GAIN_TRIM].value);
        1);

    const float gain = 1 + 4 * LookupTable<float>::lookup(*audioTaper, gainInput, false);
    const float staging = TBase::params[STAGING_PARAM].value;
    const float spread = TBase::params[SPREAD_PARAM].value;
   
    for (int i = 0; i < 2; ++i) {
        DSPImp& imp = dsp[i];
        imp.isActive = TBase::inputs[L_AUDIO_INPUT + i].active && TBase::outputs[L_AUDIO_OUTPUT + i].active;
        if (imp.isActive) {
            imp._f.setFreqSpread(spread);
            imp._f.setEdge(staging);
            imp._f.setGain(gain);
            imp._f.setVoicing(voicing);
            imp._f.setType(type);
            imp._f.setFeedback(res);
            imp._f.setNormalizedFc(fcClipped);
        }
    }
}

template <class TBase>
inline void Filt<TBase>::step()
{
    div.step();
    for (int i = 0; i < 2; ++i) {
        DSPImp& imp = dsp[i];
        if (imp.isActive) {
            const float input = TBase::inputs[L_AUDIO_INPUT+i].value;
            imp._f.run(input);
            const float output = imp._f.getOutput();
            TBase::outputs[L_AUDIO_OUTPUT+i].value = output;
        }
    }
}

template <class TBase>
int FiltDescription<TBase>::getNumParams()
{
    return Filt<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config FiltDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Filt<TBase>::FC_PARAM:
            ret = {-5.0f, 5.0f, 0, "Cutoff Freq"};
            break;
        case Filt<TBase>::Q_PARAM:
            ret = {0, 4.0f, 0, "Resonance"};
            break;
        case Filt<TBase>::TYPE_PARAM:
            ret = {0, 9.0f, 0, "Type"};
            break;
        case Filt<TBase>::DRIVE_PARAM:
            ret = {-5, 5, 0, "Drive"};
            break;
        case Filt<TBase>::STAGING_PARAM:
            ret = {0, 1, .5, "Edge"};
            break;
        case Filt<TBase>::VOICING_PARAM:
            {
                int numV = (int) LadderFilter<float>::Voicing::NUM_VOICINGS;
                ret = {0, float(numV - 1) , 0, "Voicing"};
            }
            break;
        case Filt<TBase>::SPREAD_PARAM:
            ret = {0, 1, 0, "Capacitor"};
            break;
        case Filt<TBase>::POLES_PARAM:
            ret = {0, 3, 0, "Poles"};
            break;
        case Filt<TBase>::BASS_MAKEUP_PARAM:
            ret = {-5, 5, -5, "Bass"};
            break;
        case Filt<TBase>::BASS_MAKEUP_TYPE_PARAM:
            ret = {0, 2, 1, "Bass Makeup"};
            break;
        default:
            assert(false);
    }
    return ret;
}




