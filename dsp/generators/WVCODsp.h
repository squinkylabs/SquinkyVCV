
#pragma once

#include "simd.h"
#include "SimdBlocks.h"
#include "IIRDecimator.h"

/**
 * SIMD FM VCO block
 * implements 4 VCOs
 */
class WVCODsp
{
public:
    enum class WaveForm {Sine, Fold, SawTri};
    
    static const int oversampleRate = 4;
    float_4 buffer[oversampleRate];
    float_4 lastOutput = float_4(0);
    WVCODsp() {
        downsampler.setup(oversampleRate);
    }

#ifdef _OPTSIN
    float_4 stepSin() {
       //printf("stepsin opt\n");
#if 1
        int bufferIndex;

        __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};
        float_4 phaseMod = (feedback * lastOutput);
        phaseMod += fmInput;

    
        for (bufferIndex = 0; bufferIndex < oversampleRate; ++bufferIndex) {
            phaseAcc += normalizedFreq;
            phaseAcc = SimdBlocks::wrapPhase01(phaseAcc);
            float_4 phase = SimdBlocks::wrapPhase01(phaseAcc + phaseMod);
            float_4 s = SimdBlocks::sinTwoPi(phase * twoPi);
            s *= 5;
            buffer[bufferIndex] = s;
        }
        float_4 finalSample = downsampler.process(buffer);
        lastOutput = finalSample;
        return finalSample * outputLevel;
    #else   
        return 0;
    #endif
    }
#endif

    void doSync(float_4 syncValue, int32_4& syncIndex)   {
        if (syncEnabled) {
            float_4 syncCrossing = float_4::zero();
            syncValue -= float_4(0.01f);
            const float_4 syncValueGTZero = syncValue > float_4::zero();
            const float_4 lastSyncValueLEZero = lastSyncValue <= float_4::zero();
            simd_assertMask(syncValueGTZero);
            simd_assertMask(lastSyncValueLEZero);

            const float_4 justCrossed = syncValueGTZero & lastSyncValueLEZero;
            simd_assertMask(justCrossed);
            int m = rack::simd::movemask(justCrossed);

            // If any crossed
            if (m) {
                float_4 deltaSync = syncValue - lastSyncValue;
                syncCrossing = float_4(1.f) - syncValue / deltaSync;
                syncCrossing *= float_4(oversampleRate);
                //syncIndex = syncCrossing;
                float_4 syncIndexF = SimdBlocks::ifelse(justCrossed, syncCrossing, float_4(-1));
                syncIndex = syncIndexF;
            }
            // now make a 
            lastSyncValue = syncValue;
        }
    }
    
    float_4 step(float_4 syncValue) {

#ifdef _OPTSIN
        if (!syncEnabled &&  (waveform == WaveForm::Sine)) {
            return stepSin();
        }
#endif
       // printf("not doing ops. sy=%d wv = %d\n", syncEnabled, waveform);
      //  bool synced = false;
        int32_4 syncIndex = int32_t(-1); // Index in the oversample loop where sync occurs [0, OVERSAMPLE)
        doSync(syncValue, syncIndex);

        float_4 phaseMod = (feedback * lastOutput);
        phaseMod += fmInput;

        for (int i=0; i< oversampleRate; ++i) {
    
            float_4 syncNow =  float_4(syncIndex) == float_4::zero();
            simd_assertMask(syncNow);

            stepOversampled(i, phaseMod, syncNow);
            // buffer[i] = 0;
            syncIndex -= int32_t(1);
        }
        if (oversampleRate == 1) {
            return buffer[0] * outputLevel;
        } else {
            float_4 finalSample = downsampler.process(buffer);
            // printf("dsp step using output level %s\n", toStr(outputLevel).c_str());
            finalSample += waveformOffset;
            lastOutput = finalSample;
            return finalSample * outputLevel;
        }
    }

    void stepOversampled(int bufferIndex, float_4 phaseModulation, float_4 syncNow)
    {
        float_4 s;
        phaseAcc += normalizedFreq;
        phaseAcc = SimdBlocks::wrapPhase01(phaseAcc);
        phaseAcc = SimdBlocks::ifelse(syncNow, float_4::zero(), phaseAcc);

        float_4 twoPi (2 * 3.141592653589793238);

        float_4 phase = SimdBlocks::wrapPhase01(phaseAcc + phaseModulation);
        if (waveform == WaveForm::Fold) {

            s = SimdBlocks::sinTwoPi(phase * twoPi);
            s *= correctedWaveShapeMultiplier;
            s = SimdBlocks::fold(s);
        } else if (waveform == WaveForm::SawTri) {
            float_4 k = correctedWaveShapeMultiplier;
            float_4 x = phase;
            simd_assertGE(x, float_4(0));
            simd_assertLE(x, float_4(1));
            s = SimdBlocks::ifelse( x < k, x * aLeft,  aRight * x + bRight);
        } else if (waveform == WaveForm::Sine) {
            s = SimdBlocks::sinTwoPi(phase * twoPi);
        } else {
            s = 0;
        }

        buffer[bufferIndex] = s;
    }

    void setSyncEnable(bool f) {
        syncEnabled = f;
        // printf("set sync enabled to %d\n", syncEnabled);
    }

    // public variables. The composite will set these on us,
    // and we will use them to generate audio.

    // f / fs
    float_4 normalizedFreq = float_4::zero();
    float_4 fmInput = float_4::zero();

    WaveForm waveform;
    float_4 correctedWaveShapeMultiplier = 1;
    
    float_4 aRight = 0;            // y = ax + b for second half of tri
    float_4 bRight = 0;
    float_4 aLeft = 0;
    float_4 feedback = 0;
    float_4 outputLevel = 1;
    float_4 waveformOffset = 0;

private:
    float_4 phaseAcc = float_4::zero();
    float_4 lastSyncValue = float_4::zero();
    IIRDecimator<float_4> downsampler;

    bool syncEnabled = false;
};
