#pragma once


// Need to make this compile in MS tools for unit tests
#if defined(_MSC_VER)
#define __attribute__(x)

#pragma warning (push)
#pragma warning ( disable: 4244 4267 )
#endif

#include "dsp/minblep.hpp"
#include "dsp/filter.hpp"

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
    enum class Waveform {Sin, Square, Saw, Tri, Even, END };

    void step();

    void setNormalizedFreq(float f)
    {
        normalizedFreq = std::clamp(f, 1e-6f, 0.5f);
    }
    void enableWaveform(Waveform wf, bool flag);
   
    float getWaveform() const
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
    float output = 0;
    bool doSaw = false;
    bool doEven = false;
    bool doTri = false;
    bool doSquare = false;
    bool doSin = false;

    float phase = 0.0;
    float normalizedFreq = 0;
    SyncCallback syncCallback = nullptr;

    bool gotSyncCallback = false;
    float syncCallbackCrossing = 0;

    /**
     * It's merely a convenience that we use the waveforms enum for dispatcher.
     * Could use anything.
     */
    Waveform dispatcher = Waveform::Saw;
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
    switch (dispatcher) {
        case  Waveform::Saw:
            step_saw();
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
#if 0
    static float cMax = -100;
    static float cMin = 100;
    cMin = std::min(masterPhase, cMin);
    cMax = std::max(masterPhase, cMax);

   // printf("sawSync ph=%.2f, delta=%.2f cross=%.2f (%.2f, %.2f)\n", phase, normalizedFreq, masterPhase, cMin, cMax);


   // printf("on master sync callback\n");
    // pretty sure this calc is wrong
  //  float crossing = -masterPhase / normalizedFreq;
    float crossing = masterPhase;
    float jump = -2.f * phase;


    sawMinBLEP.jump(crossing, jump);

    phase = 0;
   // printf("set phase to zero in slave\n");

    // I don't think we need to do this???
    float saw = -1.0 + 2.0*phase;
    saw += sawMinBLEP.shift();
#endif
}

inline void MinBLEPVCO::step_saw()
{
    if (gotSyncCallback) {
        gotSyncCallback = false;

        float newPhase = .5;
        float jump = -2.f * (phase - newPhase);
        this->phase = newPhase;

       // sawMinBLEP.jump(syncCallbackCrossing, jump);
        syncMinBLEP.jump(syncCallbackCrossing, jump);
        //float saw = -1.0 + 2.0*phase;
       // saw += sawMinBLEP.shift();
       // return;
    }
    phase += normalizedFreq;
   // printf("stepsaw %p, phase=%.2f normFreq=%.2f\n", this, phase, normalizedFreq);
    // Reset phase if at end of cycle
    if (phase >= 1.0) {
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;

        sawMinBLEP.jump(crossing, -2.0);
        if (syncCallback) {
           // printf("about to make sync callback\n"); fflush(stdout);
            syncCallback(crossing, -2.0f);

        }
    }

    float saw = -1.0 + 2.0*phase;
    saw += sawMinBLEP.shift();
    saw += syncMinBLEP.shift();
  //  TBase::outputs[SAW_OUTPUT].value = 5.0*saw;
    output = 5.0*saw;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

