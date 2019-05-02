
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
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
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
   
    LadderFilter<T> _f;
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
    const T normFc = fc * TBase::engineGetSampleTime();
    T fcClipped = std::min(normFc, T(.48));
    fcClipped = std::max(normFc, T(.0000001));
    _f.setNormalizedFc(fcClipped);

    const float res = TBase::params[Q_PARAM].value;
    _f.setFeedback(res);

    LadderFilter<T>::Types type = (LadderFilter<T>::Types) (int) std::round(TBase::params[TYPE_PARAM].value);
    _f.setType(type);

    LadderFilter<T>::Voicing voicing = (LadderFilter<T>::Voicing) (int) std::round(TBase::params[VOICING_PARAM].value);
    _f.setVoicing(voicing);

    //********* now the drive 
        // 0..1
    float  gainInput = scaleGain(
     //   TBase::inputs[INPUT_GAIN].value,
        0,
        TBase::params[DRIVE_PARAM].value,
       // TBase::params[PARAM_GAIN_TRIM].value);
        1);

    const float gain = 5 * LookupTable<float>::lookup(*audioTaper, gainInput, false);
    _f.setGain(gain);

    const float staging = TBase::params[STAGING_PARAM].value;
    _f.setEdge(staging);

    const float spread = TBase::params[SPREAD_PARAM].value;
    _f.setFreqSpread(spread);
}

template <class TBase>
inline void Filt<TBase>::step()
{
    div.step();
    float input = TBase::inputs[AUDIO_INPUT].value;
    _f.run(input);
    float output = _f.getOutput();
    TBase::outputs[AUDIO_OUTPUT].value = output;
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
            ret = {0, 1, 0, "Edge"};
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
            ret = {-5, 5, -5, "Base"};
            break;
        default:
            assert(false);
    }
    return ret;
}




