
#pragma once

#include "Divider.h"
#include "IComposite.h"
#include "LadderFilter.h"
#include "LadderFilterBank.h"
#include "LookupTable.h"
#include "ObjectCache.h"
#include "PeakDetector.h"

#include <assert.h>
#include <memory>
#include <string>

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;

/**
 */
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
 * with all the features: 145
 * final version: 152
 */
template <class TBase>
class Filt : public TBase
{
public:
    using T = double;
    Filt(Module * module) : TBase(module)
    {
    }
    Filt() : TBase()
    {
    }
    virtual ~Filt()
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
        FC1_TRIM_PARAM,
        FC2_TRIM_PARAM,
        Q_PARAM,
        Q_TRIM_PARAM,
        TYPE_PARAM,
        DRIVE_PARAM,
        DRIVE_TRIM_PARAM,
        VOICING_PARAM,
        EDGE_PARAM,
        SPREAD_PARAM,
        SLOPE_PARAM,
        SLOPE_TRIM_PARAM,
        BASS_MAKEUP_PARAM,
        MASTER_VOLUME_PARAM,
        EDGE_TRIM_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        L_AUDIO_INPUT,          // In poly mode, this is the only input we care about.
        R_AUDIO_INPUT,      
        CV_INPUT1,
        CV_INPUT2,
        Q_INPUT,
        DRIVE_INPUT,
        SLOPE_INPUT,
        EDGE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        L_AUDIO_OUTPUT,         // In poly mode, this is the only output we care about.
        R_AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        SLOPE0_LIGHT,
        SLOPE1_LIGHT,
        SLOPE2_LIGHT,
        SLOPE3_LIGHT,
        VOL0_LIGHT,
        VOL1_LIGHT,
        VOL2_LIGHT,
        VOL3_LIGHT,
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

    float getLevel() const
    {
        return peak.get();
    }

    void _dump(int channel, const std::string& s) {
        filters._dump(channel, s);
    }

private:


    void stepn(int);
    LadderFilterBank<T> filters;
    Divider div;
    PeakDetector peak;
   // bool isStereo = false;
   LadderFilterBank<T>::Modes mode = LadderFilterBank<T>::Modes::normal;
};


template <class TBase>
inline void Filt<TBase>::init()
{
    div.setup(4, [this] {
        this->stepn(div.getDiv());
        });
}

template <class TBase>
inline void Filt<TBase>::stepn(int divFactor)
{
    int numChannels = std::max<int>(1, TBase::inputs[L_AUDIO_INPUT].channels);
    switch (mode) {
        case LadderFilterBank<T>::Modes::normal:
            break;
        case LadderFilterBank<T>::Modes::stereo:
            if (numChannels == 1) {                   // only do ste
                numChannels = 2;
            }
            break;
        default:
            assert(false);
    }

/*
  enum class Modes {
        normal,         // mono, poly. R out == L out
        stereo,         // in L -> out L, in R -> out R
        leftOnly,       // in L -> out L, out R
        rightOnly       // in R -> out L, out R
    };
    */

    const bool li = TBase::inputs[L_AUDIO_INPUT].isConnected();
    const bool ri = TBase::inputs[R_AUDIO_INPUT].isConnected();
    const bool lo = TBase::outputs[L_AUDIO_OUTPUT].isConnected();
    const bool ro = TBase::outputs[R_AUDIO_OUTPUT].isConnected();

    // Decode the modes. If normaly number of channels is not 1, it can't be 
     mode = LadderFilterBank<T>::Modes::normal;
    if (numChannels == 1) {
        if (TBase::inputs[L_AUDIO_INPUT].isConnected() &&
            TBase::inputs[R_AUDIO_INPUT].isConnected() &&
            TBase::outputs[L_AUDIO_OUTPUT].isConnected() &&
            TBase::outputs[R_AUDIO_OUTPUT].isConnected()) {
                mode =  LadderFilterBank<T>::Modes::stereo;
        } else if (TBase::inputs[L_AUDIO_INPUT].isConnected() &&
            !TBase::inputs[R_AUDIO_INPUT].isConnected() &&
            TBase::outputs[L_AUDIO_OUTPUT].isConnected() &&
            TBase::outputs[R_AUDIO_OUTPUT].isConnected()) {
                 mode =  LadderFilterBank<T>::Modes::leftOnly;
        } else if (!TBase::inputs[L_AUDIO_INPUT].isConnected() &&
            TBase::outputs[!R_AUDIO_INPUT].isConnected() &&
            TBase::outputs[L_AUDIO_OUTPUT].isConnected() &&
            TBase::outputs[R_AUDIO_OUTPUT].isConnected()) {
                mode =  LadderFilterBank<T>::Modes::rightOnly;
        }       
    }

  //  isStereo = (numChannels == 1) &&
  //      (TBase::outputs[L_AUDIO_OUTPUT].isConnected()) &&
   //     (TBase::outputs[R_AUDIO_OUTPUT].isConnected());

    const LadderFilter<T>::Types type = (LadderFilter<T>::Types) (int) std::round(TBase::params[TYPE_PARAM].value);
    const LadderFilter<T>::Voicing voicing = (LadderFilter<T>::Voicing) (int) std::round(TBase::params[VOICING_PARAM].value);

    filters.stepn( TBase::engineGetSampleTime(), numChannels,
        TBase::inputs[CV_INPUT1], TBase::inputs[CV_INPUT2], TBase::inputs[Q_INPUT], TBase::inputs[DRIVE_INPUT],
        TBase::inputs[EDGE_INPUT], TBase::inputs[SLOPE_INPUT],
        TBase::params[FC_PARAM].value, TBase::params[FC1_TRIM_PARAM].value,  TBase::params[FC2_TRIM_PARAM].value,
        TBase::params[MASTER_VOLUME_PARAM].value,
        TBase::params[Q_PARAM].value, TBase::params[Q_TRIM_PARAM].value, TBase::params[BASS_MAKEUP_PARAM].value,
        type, voicing,
        TBase::params[DRIVE_PARAM].value,  TBase::params[DRIVE_TRIM_PARAM].value,
        TBase::params[EDGE_PARAM].value,  TBase::params[EDGE_TRIM_PARAM].value,
        TBase::params[SLOPE_PARAM].value, TBase::params[SLOPE_TRIM_PARAM].value,
        TBase::params[SPREAD_PARAM].value
    );
  

    // now update level LEDs
    peak.decay(divFactor * TBase::engineGetSampleTime() * 5);
    const float level = peak.get();
    TBase::lights[Filt<TBase>::VOL3_LIGHT].value = (level >= 7) ? .8f : .2f;
    TBase::lights[Filt<TBase>::VOL2_LIGHT].value = (level >= 3.5) ? .8f : .2f;
    TBase::lights[Filt<TBase>::VOL1_LIGHT].value = (level >= 1.75) ? .8f : .2f;
    TBase::lights[Filt<TBase>::VOL0_LIGHT].value = (level >= .87) ? .8f : .2f;


    // the main inputs and outpus are polyphonic.
    // copy the channel number
  
    TBase::outputs[L_AUDIO_OUTPUT].setChannels(numChannels);
   // TBase::outputs[R_AUDIO_OUTPUT].setChannels(numChannels);
    
}

template <class TBase>
inline void Filt<TBase>::step()
{

    div.step();

    int numChannels = TBase::inputs[L_AUDIO_INPUT].channels;

    SqInput* inputForChannel0 = nullptr;
    SqInput* inputForChannel1 = nullptr;
    switch (mode) {
        case LadderFilterBank<T>::Modes::normal:
            break;
        case LadderFilterBank<T>::Modes::stereo:
            if (numChannels == 1) {                   // only do stereo if left hooked up mono
                numChannels = 2;
                inputForChannel1 = &TBase::inputs[R_AUDIO_INPUT];
            }
            break;
        default:
            assert(false);
    }

    filters.step(numChannels, mode,
        TBase::inputs[L_AUDIO_INPUT],  TBase::outputs[L_AUDIO_OUTPUT],
        inputForChannel0, inputForChannel1);

    // if audio, clear out 
    if (numChannels == 0) {
        for (int i = 0; i < TBase::outputs[L_AUDIO_OUTPUT].channels; ++i) {
            TBase::outputs[L_AUDIO_OUTPUT].setVoltage(0, i);
        }
    }

    switch (mode) {
    case LadderFilterBank<T>::Modes::normal:
        TBase::outputs[R_AUDIO_OUTPUT].setVoltage(0, 0);
        break;
    case LadderFilterBank<T>::Modes::stereo:
        // copy the r output from poly port to  mono R out
    {
        const float r = TBase::outputs[L_AUDIO_OUTPUT].getVoltage(1);
        TBase::outputs[R_AUDIO_OUTPUT].setVoltage(r, 0);
    }
        break;
    default:
        assert(false);
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
            ret = {-5, 5, -5, "Resonance"};
            break;
        case Filt<TBase>::TYPE_PARAM:
            //ret = {0, 9.0f, 0, "Type"};
            { 
                int num = (int) LadderFilter<float>::Types::NUM_TYPES;
                ret = {0, float(num - 1) , 0, "Type"};
            }
            break;
        case Filt<TBase>::DRIVE_PARAM:
            ret = {-5, 5, -5, "Drive"};
            break;
        case Filt<TBase>::EDGE_PARAM:
            ret = {-5, 5, 0, "Edge"};
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
        case Filt<TBase>::SLOPE_PARAM:
            ret = {-5, 5, 5, "Slope"};
            break;
        case Filt<TBase>::BASS_MAKEUP_PARAM:
            ret = {0, 1, 0, "Bass"};
            break;

        case Filt<TBase>::FC1_TRIM_PARAM:
            ret = {-1, 1, 0, "Fc 1 trim"};
            break;
        case Filt<TBase>::FC2_TRIM_PARAM:
            ret = {-1, 1, 0, "Fc 2 trim"};
            break;
        case Filt<TBase>::DRIVE_TRIM_PARAM:
            ret = {-1, 1, 0, "Drive trim"};
            break;
        case Filt<TBase>::Q_TRIM_PARAM:
            ret = {-1, 1, 0, "Q trim"};
            break;
        case Filt<TBase>::SLOPE_TRIM_PARAM:
            ret = {-1, 1, 0, "Slope trim"};
            break;   
        case Filt<TBase>::MASTER_VOLUME_PARAM:
            ret = {0, 1, .5, "Output volume"};
            break;
        case Filt<TBase>::EDGE_TRIM_PARAM:
            ret = {-1, 1, 0, "Edge trim"};
            break;
            
#if 0
        case Filt<TBase>::BASS_MAKEUP_TYPE_PARAM:
            ret = {0, 2, 1, "Bass Makeup"};
            break;
#endif
        default:
            assert(false);
    }
    return ret;
}






