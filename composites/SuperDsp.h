
#pragma once


#include "IIRDecimator.h"
#include "NonUniformLookupTable.h"



class SawtoothDetuneCurve
{
public:
    /**
    * @param depth is "detune" knob. 0..1
    * returns a number such that freq = detuneFactor * initialFreq
    */
    float getDetuneFactor(float depth)
    {
        return NonUniformLookupTable<float>::lookup(table, depth);
    }

    SawtoothDetuneCurve()
    {
        // this data is pretty regular - could use uniform table
        using T = NonUniformLookupTable<float>;
        T::addPoint(table, 0, 0);
        T::addPoint(table, .0551f, .00967f);
        T::addPoint(table, .118f, .022f);
        T::addPoint(table, .181f, .04f);
        T::addPoint(table, .244f, .0467f);
        T::addPoint(table, .307f, .059f);

        T::addPoint(table, .37f, .0714f);
        T::addPoint(table, .433f, .0838f);
        T::addPoint(table, .496f, .0967f);
        T::addPoint(table, .559f, .121f);
        T::addPoint(table, .622f, .147f);
        T::addPoint(table, .748f, .243f);
        T::addPoint(table, .811f, .293f);
        T::addPoint(table, .874f, .343f);
        T::addPoint(table, .937f, .392f);
        T::addPoint(table, 1, 1);
        NonUniformLookupTable<float>::finalize(table);
    }
private:
    NonUniformLookupTableParams<float> table;
};


/**
 * the signal processing for one channel
 * of saws
 */
class SuperDsp
{
public:
    void init();
    /**
     * divisor is 4 for 4X oversampling, etc.
     */
    void setupDecimationRatio(int divisor);

   // void updatePhaseInc();
    void updatePhaseInc(int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
        float fmParam, float detuneInput, float detuneParam, float detuneTrimParam );
    void updateHPFilters();
    void updateMix();
    void updateStereo(); 
    void updateStereoGains();
private:
    IIRDecimator decimatorLeft;
    IIRDecimator decimatorRight;

   

    static const int numSaws = 7;
    float globalPhaseInc=0;
    float phase[numSaws] = {0};
    float phaseInc[numSaws] = {0};

    // current left and right gains
    float sawGainsStereo[2][numSaws] = {
        {.2f},
        {.2f}
    };

    float sawGainsNorm[2][numSaws] = {
        {1.f, .26f, .87f, .71f, .5f, .97f, 0.f},
        {0.f, .97f, .5f,  .71f, .87f,.26f, 1.f}
    };

    float sawGainsHardPan[2][numSaws] = {
        {1.1f, 0.f, 1.1f, 1.f, 0.f, 1.1f, 0.f},
        {0.f, 1.1f, 0.f,  1.f, 1.1f, 0.f, 1.1f}
    };

        // TODO: make static
    float const detuneFactors[numSaws] = {
        .89f,
        .94f,
        .98f,
        1.f,
        1.02f,
        1.06f,
        1.107f
    };



    // TODO: we don't need 16 of these!!!
    // this could all be global
    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();
     AudioMath::ScaleFun<float> scaleDetune;
    SawtoothDetuneCurve detuneCurve;
  
};

inline void SuperDsp::init()
{
    scaleDetune = AudioMath::makeLinearScaler<float>(0, 1);
}

inline void SuperDsp::setupDecimationRatio(int decimateDiv)
{
    decimatorLeft.setup(decimateDiv);
    decimatorRight.setup(decimateDiv);
}

inline void SuperDsp:: updatePhaseInc(int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
        float fmParam, float detuneCVInput, float detuneParam, float detuneTrimParam )
{
   // const float cv = TBase::inputs[CV_INPUT].getVoltage(0);

    const float finePitch = fineTuneParam / 12.0f;
    const float semiPitch = semiParam / 12.0f;

    float pitch = 1.0f + roundf(octaveParam) +
        semiPitch +
        finePitch;

    pitch += cv;

    const float fm = fmInput;
    const float fmDepth = AudioMath::quadraticBipolar(fmParam);

    pitch += (fmDepth * fm);

    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    pitch += q;
    const float freq = expLookup(pitch);
    globalPhaseInc = sampleTime * freq;

    const float rawDetuneValue = scaleDetune(
        detuneCVInput,
        detuneParam,
        detuneTrimParam);

    const float detuneInput = detuneCurve.getDetuneFactor(rawDetuneValue);


   // const bool classic = TBase::params[CLEAN_PARAM].value < .5f;
   // const int oversampleRate = oversampleRate();

    for (int i = 0; i < numSaws; ++i) {
        float detune = (detuneFactors[i] - 1) * detuneInput;
        detune += 1;
        float phaseIncI = globalPhaseInc * detune;
        phaseIncI = std::min(phaseIncI, .4f);         // limit so saws don't go crazy
        if (oversampleRate > 1) {
            phaseIncI /= oversampleRate;
        }
        phaseInc[i] = phaseIncI;
    }
}

inline void SuperDsp:: updateHPFilters()
{
    assert(false);
}

inline void SuperDsp:: updateMix()
{
    assert(false);
}

inline void SuperDsp:: updateStereo()
{
    assert(false);
}
 
inline void SuperDsp::updateStereoGains()
{
    assert(false);
}


// The processing params that can be 
// shared between all dsp blocks
class SuperDspCommon
{
public:

    void init();

    static const unsigned int MAX_OVERSAMPLE = 16;
     /**
     * divisor is 4 for 4X oversampling, etc.
     */
    void setupDecimationRatio(int divisor);

    /**
     * called every sample to calc audio.
     */
    void step();

     /**
     * called every 'nn' sample to calc CV.
     */

    void stepn(int n, int index, int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
        float fmParam, float detuneInput, float detuneParam, float detuneTrimParam );

     int numChannels = 1;
private:
    float bufferLeft[MAX_OVERSAMPLE] = {0};
    float bufferRight[MAX_OVERSAMPLE] = {0};

    SuperDsp dsp[16];   // maximum 16 channels

    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();
    

   // using lambda = std::function<void(SuperDsp&)>;
  //  void forEach(lambda);
};


inline void SuperDspCommon::init()
{
    for (int i=0; i<numChannels; ++i) {
        dsp[i].init();
    }    
}

inline void SuperDspCommon::setupDecimationRatio(int divisor)
{
    for (int i=0; i<numChannels; ++i) {
        dsp[i].setupDecimationRatio(divisor);
    }
}

inline  void SuperDspCommon::step()
{
 assert(false);
}

inline  void SuperDspCommon::stepn(int n, int index, int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
        float fmParam, float detuneInput, float detuneParam, float detuneTrimParam)
{
    SuperDsp& d = dsp[index];

    //     void updatePhaseInc(int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
   //     float fmParam, float detuneInput, float detuneParam, float detuneTrimParam );
    d.updatePhaseInc(oversampleRate, sampleTime, cv, fineTuneParam, semiParam, octaveParam, fmInput, fmParam, detuneInput, detuneParam, detuneTrimParam);
    d.updateHPFilters();
    d.updateMix();
    d.updateStereo(); 
    d.updateStereoGains();

}
