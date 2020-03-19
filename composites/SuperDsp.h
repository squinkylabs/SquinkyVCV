
#pragma once


#include "IIRDecimator.h"
#include "NonUniformLookupTable.h"

/**
 * This awful hack is so that both the real plugin and
 * the unit tests can pass this "Output" struct around
 */
#ifdef __PLUGIN
namespace rack {
namespace engine {
struct Input;
struct Param;
struct Output;
}
}  // namespace rack
#else
#include "TestComposite.h"
#endif



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
#ifdef __PLUGIN
   // using Param = rack::engine::Param;
   // using Input = rack::engine::Input;
    using Output = rack::engine::Output;
#else
    using Output = ::Output;
    //using Input = ::Input;
    //using Param = ::Param;
#endif
    void init();
    /**
     * divisor is 4 for 4X oversampling, etc.
     */
    void setupDecimationRatio(int divisor);

    void step(int channel, int oversampleRate, bool isStereo, float* bufferLeft, float* bufferRight, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut);

   // void updatePhaseInc();
    void updatePhaseInc(int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
        float fmParam, float detuneInput, float detuneParam, float detuneTrimParam );
    void updateHPFilters();
    void updateMix(float mixInput, float mixParam, float mixTrimParam);
    void updateStereo(); 
    void updateStereoGains();
private:
    IIRDecimator decimatorLeft;
    IIRDecimator decimatorRight;

   
   // int m_oversampleRate = 1;
    static const int numSaws = 7;
    float globalPhaseInc=0;
    float phase[numSaws] = {0};
    float phaseInc[numSaws] = {0};

    float gainCenter = 0;
    float gainSides = 0;

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

    void updateAudioClassic(int channel, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut);
    void updateAudioClean(int channel, float* buffer, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut, int oversampleRate);
    void updateAudioClassicStereo();
    void updateAudioCleanStereo();
    void runSaws(float& left);
    void runSawsStereo(float& left, float& right);
  
};

inline void SuperDsp::init()
{
    scaleDetune = AudioMath::makeLinearScaler<float>(0, 1);
}

// is this function even necessary? I think it's just initial setup
inline void SuperDsp::setupDecimationRatio(int decimateDiv)
{
    decimatorLeft.setup(decimateDiv);
    decimatorRight.setup(decimateDiv);
}

inline void SuperDsp::step(int channel, int oversampleRate, bool isStereo, float* bufferLeft, float* bufferRight, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut)
{
    assert(oversampleRate == 1);
    printf("step, overs=%d st=%d\n", oversampleRate, isStereo);
    if ((oversampleRate == 1) && !isStereo) {
        updateAudioClassic(channel, leftOut, rightOut);
    } else if ((oversampleRate == 1) && isStereo) {
        updateAudioClassicStereo();
    } else if ((oversampleRate != 1) && !isStereo) {
        updateAudioClean(channel, bufferLeft, leftOut, rightOut, oversampleRate);
    } else {
        updateAudioCleanStereo();
    }
}

inline void SuperDsp::updateAudioClassic(int channel, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut)
{
    float left;
    runSaws(left);

    // TODO: put back hpf !!!!!
   // const float outputLeft = hpfLeft.run(left);
    const float output = left;
    leftOut.setVoltage(output, channel);
    rightOut.setVoltage(output, channel);
}

inline void SuperDsp::updateAudioClean(int channel, float* buffer, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut, int oversampleRate)
{
    const int bufferSize = oversampleRate;
    decimatorLeft.setup(bufferSize);
    assert(false);
    for (int i = 0; i < bufferSize; ++i) {
        float left;
        runSaws(left);
        buffer[i] = left;
    }

    const float output = decimatorLeft.process(buffer);
    leftOut.setVoltage(output, channel);
    rightOut.setVoltage(output, channel);
    printf("clean\n");
}

inline void SuperDsp::updateAudioClassicStereo()
{
    printf("update audio cs\n");
     assert(false);
}

inline void SuperDsp::updateAudioCleanStereo()
{
    printf("update audio clean s\n");
     assert(false);
}


inline void SuperDsp::runSaws(float& left)
{
    float mix = 0;
    for (int i = 0; i < numSaws; ++i) {
        assert(phaseInc[i] > 0 && phaseInc[i] < .1);        // just for debugging
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        assert(phase[i] <= 1);
        assert(phase[i] >= 0);

        const float gain = (i == numSaws / 2) ? gainCenter : gainSides;
        assert(gain > 0);       // just for debugging?
        mix += (phase[i] - .5f) * gain;        // experiment to get rid of DC

    }

    mix *= 4.5;       // too low 2 too high 10
    left = mix;
}

inline void SuperDsp::runSawsStereo(float& left, float& right)
{
    left = right = 0;
    for (int i = 0; i < numSaws; ++i) {
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        assert(phase[i] <= 1);
        assert(phase[i] >= 0);

        left +=  (phase[i] - .5f) *sawGainsStereo[0][i];
        right +=  (phase[i] - .5f) *sawGainsStereo[1][i];
    }
}

inline void SuperDsp::updatePhaseInc(int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
        float fmParam, float detuneCVInput, float detuneParam, float detuneTrimParam )
{
   // const float cv = TBase::inputs[CV_INPUT].getVoltage(0);

    const float finePitch = fineTuneParam / 12.0f;
    const float semiPitch = semiParam / 12.0f;

    float pitch = 1.0f + roundf(octaveParam) +
        semiPitch +
        finePitch;

    pitch += cv;
   // printf("initial cv = %.2f from fine=%.2f, semi=%.2f, oct=%.2f cv = %.2f\n", pitch, finePitch, semiPitch, octaveParam, cv );

    const float fm = fmInput;
    const float fmDepth = AudioMath::quadraticBipolar(fmParam);

    pitch += (fmDepth * fm);

    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    pitch += q;
    const float freq = expLookup(pitch);
  //  printf("in update, final freq = %.2f from pitch cv of %.2f\n", freq, pitch); fflush(stdout);
    globalPhaseInc = sampleTime * freq;
    assert(sampleTime < .01);
     assert(globalPhaseInc > 0 && globalPhaseInc < .4);      // just for debuggin

  //  printf("final global phase inc = %f sampleRate = %f\n", globalPhaseInc, 1.f / sampleTime);

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

        assert(globalPhaseInc > 0 && globalPhaseInc < .4);      // just for debuggin
        
        phaseIncI = std::min(phaseIncI, .4f);         // limit so saws don't go crazy
        if (oversampleRate > 1) {
            phaseIncI /= oversampleRate;
        }
        assert(phaseIncI > 0 && phaseIncI < .1);   
        phaseInc[i] = phaseIncI;
      //  printf("ph[%d] = %f\n", i, phaseIncI);
    }
}

inline void SuperDsp::updateHPFilters()
{
    #ifdef _DEBUG
    printf("todo: updateHPFilters\n");
    #endif
}

inline void SuperDsp::updateMix(float mixInput, float mixParam, float mixTrimParam)
{

    const float rawMixValue = scaleDetune(
        mixInput,
        mixParam,
        mixTrimParam);


    gainCenter = -0.55366f * rawMixValue + 0.99785f;

    gainSides = -0.73764f * rawMixValue * rawMixValue +
        1.2841f * rawMixValue + 0.044372f;

}

inline void SuperDsp::updateStereo()
{
 #ifdef _DEBUG
    printf("todo: updateStereo\n");
        #endif
}
 
inline void SuperDsp::updateStereoGains()
{
        #ifdef _DEBUG
    printf("todo: updateStereoGains\n");
    #endif
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
    void step(bool isStereo, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut, int oversampleRate);

     /**
     * called every 'n' sample to calc CV.
     * must be called for each active channel.
     */
    void stepn(int n, int index, int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
        float fmParam, float detuneInput, float detuneParam, float detuneTrimParam,
        float mixInput, float mixParam, float mixTrimPara );

   

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

inline  void SuperDspCommon::step(bool isStereo, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut, int oversampleRate)
{
     for (int i=0; i<numChannels; ++i) {
         //  void step(int channel, bool isStereo, float* bufferLeft, float* bufferRight, SuperDsp::Output& leftOut, SuperDsp::Output& rightOut);
        dsp[i].step(i, oversampleRate, isStereo, bufferLeft, bufferRight, leftOut, rightOut);
    }    
}

inline  void SuperDspCommon::stepn(int n, int index, int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
        float fmParam, float detuneInput, float detuneParam, float detuneTrimParam,
        float mixInput, float mixParam, float mixTrimParam)
{
    SuperDsp& d = dsp[index];

    //     void updatePhaseInc(int oversampleRate, float sampleTime, float cv, float fineTuneParam, float semiParam, float octaveParam, float fmInput,
   //     float fmParam, float detuneInput, float detuneParam, float detuneTrimParam );
    d.updatePhaseInc(oversampleRate, sampleTime, cv, fineTuneParam, semiParam, octaveParam, fmInput, fmParam, detuneInput, detuneParam, detuneTrimParam);
    d.updateHPFilters();
    d.updateMix(mixInput, mixParam, mixTrimParam);
    d.updateStereo(); 
    d.updateStereoGains();

}
