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
    using SyncCallback = std::function<void(float p, float dx)>;

    MinBLEPVCO();
    enum class Waveform {Sin, Tri, Saw, Square, Even, END };

    void step();

    void setNormalizedFreq(float f)
    {
        normalizedFreq = std::clamp(f, 1e-6f, 0.5f);
    }
   // void enableWaveform(Waveform wf, bool flag);
    void setWaveform(Waveform);
   
    float getOutput() const
    {
       // printf("vco.getWave will ret %f\n", waveformOutputs[(int) wf]);
      //  return waveformOutputs[(int) wf];
        return output;
    }

    /**
     * Send the sync waveform to VCO
     */
   // void syncInput(float value);
    void onMasterSync(float phase, float dx);

    void setSyncCallback(SyncCallback);


private:
   // bool waveformEnabled[Waveforms::END] = {false};
  //  float waveformOutputs[(int)Waveform::END] = {0};
#if 0
  
    bool doSaw = false;
    bool doEven = false;
    bool doTri = false;
    bool doSquare = false;
    bool doSin = false;
#endif

    float output = 0;
    Waveform waveform = Waveform::Saw;

    float phase = 0.0;
    float normalizedFreq = 0;
    SyncCallback syncCallback = nullptr;

    bool gotSyncCallback = false;
    float syncCallbackCrossing = 0;

    /** Whether we are past the pulse width already */
    bool halfPhase = false;

    /**
     * It's merely a convenience that we use the waveforms enum for dispatcher.
     * Could use anything.
     */
 //   Waveform dispatcher = Waveform::Saw;
    int loopCounter = 0;

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
    void step_all();

   // void zeroOutputsExcept(Waveform);

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

#if 0
inline void MinBLEPVCO::enableWaveform(Waveform wf, bool flag)
{
    switch (wf) {
        case Waveform::Saw:
            doSaw = flag;
            break;
        default:
            assert(flag == false);
    }
}
#endif

#if 0
inline void MinBLEPVCO::zeroOutputsExcept(Waveform  except)
{
    for (int i = 0; i < (int) Waveform::END; ++i) {
        const Waveform wf = Waveform(i);
        if (wf != except) {
            // if we do even, we do sin at same time
            if ((wf == Waveform::Sin) && (except == Waveform::Even)) {
            } else {
                waveformOutputs[i] = 0;
            }
        }
    }
}
#endif



inline void MinBLEPVCO::step()
{
  //  assert(sampleTime > 0);
   // assert(normalizedFreq > 0);
    // We don't need to look for connected outputs every cycle.
    // do it less often, and store results.
    if (--loopCounter < 0) {
        loopCounter = 16;

#if 0
        doSaw = TBase::outputs[SAW_OUTPUT].active;
        doEven = TBase::outputs[EVEN_OUTPUT].active;
        doTri = TBase::outputs[TRI_OUTPUT].active;
        doSq = TBase::outputs[SQUARE_OUTPUT].active;
        doSin = TBase::outputs[SINE_OUTPUT].active;
#endif


#if 0
        /**
         * Figure out which specialized processing function to run
         */
        if (doSaw && !doEven && !doTri && !doSquare && !doSin) {
            dispatcher = Waveform::Saw;
     //       zeroOutputsExcept(dispatcher);
        } else if (!doSaw && doEven && !doTri && !doSquare) {
            dispatcher = Waveform::Even;
      //      zeroOutputsExcept(dispatcher);
        } else if (!doSaw && !doEven && !doTri && !doSquare && doSin) {
            dispatcher = Waveform::Sin;
       //     zeroOutputsExcept(dispatcher);
        } else if (!doSaw && !doEven && doTri && !doSquare && !doSin) {
            dispatcher = Waveform::Tri;
       //     zeroOutputsExcept(dispatcher);
        } else if (!doSaw && !doEven && !doTri && doSquare && !doSin) {
            dispatcher = Waveform::Square;
         //   zeroOutputsExcept(dispatcher);
        } else {
            dispatcher = Waveform::END;
        }
   
#endif
    }

#if 0   // Move to composite
    // Compute frequency, pitch is 1V/oct
    float pitch = 1.0 + roundf(TBase::params[OCTAVE_PARAM].value) + TBase::params[TUNE_PARAM].value / 12.0;
    pitch += TBase::inputs[PITCH1_INPUT].value + TBase::inputs[PITCH2_INPUT].value;
    pitch += TBase::inputs[FM_INPUT].value / 4.0;

    // Use lookup table for pitch lookup
    const float q = float(log2(261.626));       // move up to pitch range of even vco
    pitch += q;
    _freq = expLookup(pitch);
#endif


    // Advance phase
  //  float f = (_testFreq) ? _testFreq : _freq;
 //   float deltaPhase = clamp(f * TBase::engineGetSampleTime(), 1e-6f, 0.5f);
  //  float deltaPhase = clamp(normalizedFreq)

    // call the dedicated dispatch routines for the special case waveforms.
    switch (waveform) {
        case  Waveform::Saw:
            step_saw();
            break;
        case  Waveform::Square:
            step_sq();
            break;
#if 0
        case EVEN_OUTPUT:
            step_even(deltaPhase);
            break;
        case SINE_OUTPUT:
            step_sin(deltaPhase);
            break;
        case TRI_OUTPUT:
            step_tri(deltaPhase);
            break;
        case SQUARE_OUTPUT:
            step_sq(deltaPhase);
            break;
        case NUM_OUTPUTS:
            step_all(deltaPhase);
            break;
#endif
        case Waveform::END:
            break;                  // don't do anything if no outputs
        default:
            assert(false);
    }
}

// callback from master sync when it rolls over
inline void MinBLEPVCO::onMasterSync(float masterPhase, float dx)
{

    gotSyncCallback = true;
    syncCallbackCrossing = masterPhase;
   // printf("%s get master sync callback\n", name.c_str());
}

inline void MinBLEPVCO::step_saw()
{
#ifdef _LOG
    printf("\n%s: step  \n", name.c_str());
#endif

    phase += normalizedFreq;
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = - syncCallbackCrossing * normalizedFreq;

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
    } else if (phase >= 1.0) {

        // Not sync case, regular overfloat
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;
#ifdef _LOG
        printf("%s: phase wrap %.2f->%.2f cross=%.2f jump=%.2f  \n", name.c_str(),
            phase + 1, phase, crossing, -2.0);
#endif

        sawMinBLEP.jump(crossing, -2.0);
        if (syncCallback) {
            syncCallback(crossing, -2.0f);
        }
    }

    float saw = -1.0 + 2.0*phase;
    const float mb = sawMinBLEP.shift();
    const float smb = syncMinBLEP.shift();
#ifdef _LOG
    printf("%s: final out saw=%.2f mb=%.2f smb = %.2f tot=%.2f\n",
        name.c_str(),
        saw, mb, smb, saw+mb+smb);
#endif
    saw += (mb + smb);

  //  TBase::outputs[SAW_OUTPUT].value = 5.0*saw;
    output = 5.0*saw;
}

inline void MinBLEPVCO::step_sq()
{
    phase += normalizedFreq;
    if (gotSyncCallback) {
        gotSyncCallback = false;
        assert(false);  // TODO
    }

    // Pulse width
    float pw;

    pw = .5;        // TODO: figure out pw,
   // pw = TBase::params[PWM_PARAM].value + TBase::inputs[PWM_INPUT].value / 5.0;
    const float minPw = 0.05f;
    pw = rack::rescale(std::clamp(pw, -1.0f, 1.0f), -1.0f, 1.0f, minPw, 1.0f - minPw);

    if (!halfPhase && phase >= pw) {
        float crossing = -(phase - pw) / normalizedFreq;
        squareMinBLEP.jump(crossing, 2.0);
        halfPhase = true;
    }


    // Reset phase if at end of cycle
    if (phase >= 1.0) {
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;
        squareMinBLEP.jump(crossing, -2.0);
        halfPhase = false;
    }

    float square = (phase < pw) ? -1.0 : 1.0;
    square += squareMinBLEP.shift();
    output = 5.0*square;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

