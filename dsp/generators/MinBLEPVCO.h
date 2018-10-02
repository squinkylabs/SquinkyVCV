#pragma once


// Need to make this compile in MS tools for unit tests
#if defined(_MSC_VER)
#define __attribute__(x)

#pragma warning (push)
#pragma warning ( disable: 4244 4267 )
#endif

#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif

//#include "math.hpp"


#include "dsp/minblep.hpp"
#include "dsp/filter.hpp"
#include "AudioMath.h"
#include "ObjectCache.h"

#include <functional>

// until c++17
#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif

/* VCO core using MinBLEP to reduce aliasing.
 * Originally based on Befaco EvenVCO
 */

class MinBLEPVCO
{
public:
    friend class TestMB;

    /**
     * ph is the "phase (-1..0)"
     */
    using SyncCallback = std::function<void(float p)>;

    MinBLEPVCO();
    enum class Waveform
    {
        Sin, Tri, Saw, Square, Even, END
    };

    void step();

    void setNormalizedFreq(float f, float st)
    {
        normalizedFreq = std::clamp(f, 1e-6f, 0.5f);
        sampleTime = st;
    }

    void setWaveform(Waveform);

    float getOutput() const
    {
        return output;
    }

    /**
     * Send the sync waveform to VCO.
     * usually called from outside.
     */
    void onMasterSync(float phase);
    void setSyncCallback(SyncCallback);
    void setPulseWidth(float);

private:

    float output = 0;
    Waveform waveform = Waveform::Saw;

    float phase = 0.0;
    float normalizedFreq = 0;
    float sampleTime = 0;
    SyncCallback syncCallback = nullptr;
    float tri = 0;

    bool gotSyncCallback = false;
    float syncCallbackCrossing = 0;

    /**
    * References to shared lookup tables.
    * Destructor will free them automatically.
    */

    std::shared_ptr<LookupTableParams<float>> sinLookup = {ObjectCache<float>::getSinLookup()};

    /** Whether we are past the pulse width already */
    bool halfPhase = false;

    int loopCounter = 0;        // still used?
    float pulseWidth = .5;

    rack::MinBLEP<16> triSquareMinBLEP;
    rack::MinBLEP<16> triMinBLEP;
    rack::MinBLEP<16> sineMinBLEP;
    rack::MinBLEP<16> doubleSawMinBLEP;
    rack::MinBLEP<16> sawMinBLEP;
    rack::MinBLEP<16> syncMinBLEP;
    rack::MinBLEP<16> squareMinBLEP;

    /**
     * Waveform generation helper
     */
    void step_even();
    void step_saw();
    void step_sq();
    void step_sin();
    void step_tri();

    float sineLook(float input) const;

    std::string name;
};

inline MinBLEPVCO::MinBLEPVCO()
{
    triSquareMinBLEP.minblep = rack::minblep_16_32;
    triSquareMinBLEP.oversample = 32;
    triMinBLEP.minblep = rack::minblep_16_32;
    triMinBLEP.oversample = 32;
    sineMinBLEP.minblep = rack::minblep_16_32;
    sineMinBLEP.oversample = 32;
    doubleSawMinBLEP.minblep = rack::minblep_16_32;
    doubleSawMinBLEP.oversample = 32;
    sawMinBLEP.minblep = rack::minblep_16_32;
    sawMinBLEP.oversample = 32;
    syncMinBLEP.minblep = rack::minblep_16_32;
    syncMinBLEP.oversample = 32;
    squareMinBLEP.minblep = rack::minblep_16_32;
    squareMinBLEP.oversample = 32;
}

inline void MinBLEPVCO::setSyncCallback(SyncCallback cb)
{
    assert(!syncCallback);
    syncCallback = cb;
}

inline void MinBLEPVCO::setWaveform(Waveform wf)
{
    waveform = wf;
}

inline void MinBLEPVCO::setPulseWidth(float pw)
{
    pulseWidth = pw;
}

inline void MinBLEPVCO::step()
{
    // call the dedicated dispatch routines for the special case waveforms.
    switch (waveform) {
        case  Waveform::Saw:
            step_saw();
            break;
        case  Waveform::Square:
            step_sq();
            break;
        case  Waveform::Sin:
            step_sin();
            break;
        case  Waveform::Tri:
            step_tri();
            break;
        case  Waveform::Even:
            step_even();
            break;
        case Waveform::END:
            output = 0;
            break;                  // don't do anything if no outputs
        default:
            assert(false);
    }
}

// callback from master sync when it rolls over
inline void MinBLEPVCO::onMasterSync(float masterPhase)
{
    gotSyncCallback = true;
    syncCallbackCrossing = masterPhase;
}

#if 0
static inline float sawFromPhase(float phase)
{
    return -1.0 + 2.0 * phase;
}
#endif

inline void MinBLEPVCO::step_saw()
{
    phase += normalizedFreq;
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;

        const float jump = -2.f * (phase - newPhase);

       // const float oldSaw = sawFromPhase(phase);
      //  const float newSaw = sawFromPhase(newPhase);
       // const float jump = newSaw - oldSaw;
       // printf("jump0= %.2f new=%.2f\n", jump0, jump); fflush(stdout);


#ifdef _LOG 
        printf("%s: got sync ph=%.2f nph=%.2f excess=%.2f send cross %.2f jump %.2f \n", name.c_str(),
            phase, newPhase,
            excess,
            syncCallbackCrossing, jump);
#endif
        syncMinBLEP.jump(syncCallbackCrossing, jump);
        this->phase = newPhase;
        //return;
    } else if (phase >= 1.0) {

        // Not sync case, regular overflow
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;
#ifdef _LOG
        printf("%s: phase wrap %.2f->%.2f cross=%.2f jump=%.2f  \n", name.c_str(),
            phase + 1, phase, crossing, -2.0);
#endif

        sawMinBLEP.jump(crossing, -2.0);
        if (syncCallback) {
            syncCallback(crossing);
        }
    }

    float saw = -1.0 + 2.0*phase;
   // float saw = sawFromPhase(phase);
    const float mb = sawMinBLEP.shift();
    const float smb = syncMinBLEP.shift();
#ifdef _LOG
    printf("%s: final out saw=%.2f mb=%.2f smb = %.2f tot=%.2f\n",
        name.c_str(),
        saw, mb, smb, saw + mb + smb);
#endif
    saw += (mb + smb);
    output = 5.0*saw;
}

#if 0
inline float squareFromPhase(float phase)
{
    return (phase < pulseWidth) ? -1.0f : 1.0f;
}
#endif

inline void MinBLEPVCO::step_sq()
{
    phase += normalizedFreq;
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;

       // const float jump = -2.f * (phase - newPhase); 
        const float oldOutput = phase < pulseWidth ? -1.0 : 1.0;
        const float newOutput = newPhase < pulseWidth ? -1.0 : 1.0;
        if (newOutput != oldOutput) {
            const float jump = newOutput - oldOutput;
            syncMinBLEP.jump(syncCallbackCrossing, jump);
        }
        halfPhase = newPhase < pulseWidth;
        this->phase = newPhase;
        return;
    }

    // when phase first goes above pulse width,
    // generate a blep
    if (!halfPhase && phase >= pulseWidth) {
        float crossing = -(phase - pulseWidth) / normalizedFreq;
        squareMinBLEP.jump(crossing, 2.0);
        halfPhase = true;
    }

    // Reset phase if at end of cycle
    if (phase >= 1.0) {
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;
        squareMinBLEP.jump(crossing, -2.0);
        halfPhase = false;
        if (syncCallback) {
            syncCallback(crossing);
        }
    }

    float square = (phase < pulseWidth) ? -1.0 : 1.0;
    square += squareMinBLEP.shift();
    square += syncMinBLEP.shift();
    output = 5.0*square;
}

inline float MinBLEPVCO::sineLook(float input) const
{
    // want cosine, but only have sine lookup
    float adjPhase = input + .25f;
    if (adjPhase >= 1) {
        adjPhase -= 1;
    }

    return -LookupTable<float>::lookup(*sinLookup, adjPhase, true);
}

inline void MinBLEPVCO::step_sin()
{
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;

        const float oldOutput = sineLook(phase);
        const float newOutput = sineLook(newPhase);
        const float jump = newOutput - oldOutput;

        syncMinBLEP.jump(syncCallbackCrossing, jump);
        this->phase = newPhase;
       // return;
    } else {

        phase += normalizedFreq;

        // Reset phase if at end of cycle
        if (phase >= 1.0) {
            phase -= 1.0;
            if (syncCallback) {
                float crossing = -phase / normalizedFreq;
                syncCallback(crossing);
            }
        }
    }

    float sine = sineLook(phase);
    sine += syncMinBLEP.shift();
    output = 5.0*sine;
}

inline void MinBLEPVCO::step_tri()
{
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;
        const float jump = -2.f * (phase - newPhase);
#ifdef _LOG 
        printf("%s: got sync ph=%.2f nph=%.2f excess=%.2f send cross %.2f jump %.2f \n", name.c_str(),
            phase, newPhase,
            excess,
            syncCallbackCrossing, jump);
#endif
        syncMinBLEP.jump(syncCallbackCrossing, jump);
        this->phase = newPhase;
        return;
    }
    float oldPhase = phase;
    phase += normalizedFreq;

    if (oldPhase < 0.5 && phase >= 0.5) {
        const float crossing = -(phase - 0.5) / normalizedFreq;
        triSquareMinBLEP.jump(crossing, 2.0);
    }

    // Reset phase if at end of cycle
    if (phase >= 1.0) {
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;
        triSquareMinBLEP.jump(crossing, -2.0);
        halfPhase = false;
        if (syncCallback) {
            syncCallback(crossing);
        }
    }

    // Outputs
    float triSquare = (phase < 0.5) ? -1.0 : 1.0;
    triSquare += triSquareMinBLEP.shift();
    triSquare += syncMinBLEP.shift();

    // Integrate square for triangle
    tri += 4.0 * triSquare * normalizedFreq;
    tri *= (1.0 - 40.0 * sampleTime);

    // Set output
    output = 5.0*tri;
}

inline void MinBLEPVCO::step_even()
{
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;
        const float jump = -2.f * (phase - newPhase);
#ifdef _LOG 
        printf("%s: got sync ph=%.2f nph=%.2f excess=%.2f send cross %.2f jump %.2f \n", name.c_str(),
            phase, newPhase,
            excess,
            syncCallbackCrossing, jump);
#endif
        syncMinBLEP.jump(syncCallbackCrossing, jump);
        this->phase = newPhase;
        return;
    }
    float oldPhase = phase;
    phase += normalizedFreq;

    if (oldPhase < 0.5 && phase >= 0.5) {
        float crossing = -(phase - 0.5) / normalizedFreq;
        doubleSawMinBLEP.jump(crossing, -2.0);
    }

    // Reset phase if at end of cycle
    if (phase >= 1.0) {
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;
        doubleSawMinBLEP.jump(crossing, -2.0);
        if (syncCallback) {
            syncCallback(crossing);
        }
    }

    //sine = -cosf(2*AudioMath::Pi * phase);
    // want cosine, but only have sine lookup
    float adjPhase = phase + .25f;
    if (adjPhase >= 1) {
        adjPhase -= 1;
    }
    const float sine = -LookupTable<float>::lookup(*sinLookup, adjPhase, true);

    float doubleSaw = (phase < 0.5) ? (-1.0 + 4.0*phase) : (-1.0 + 4.0*(phase - 0.5));
    doubleSaw += doubleSawMinBLEP.shift();
    doubleSaw += syncMinBLEP.shift();
    const float even = 0.55 * (doubleSaw + 1.27 * sine);

    //TBase::outputs[SINE_OUTPUT].value = 5.0*sine;
    output = 5.0*even;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

