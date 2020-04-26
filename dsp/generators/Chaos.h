#pragma once

#include "BiquadState.h"
#include "BiquadFilter.h"
#include "ButterworthFilterDesigner.h"
#include "FractionalDelay.h"
#include "LowpassFilter.h"

class ChaosGen1
{
public:
    float step() {
        assert(configured);
        const float next = g * x * (1 - x);
        x = next;
        const double result = 10 * BiquadFilter<double>::run(next, dcBlockState, dcBlockParams);
        return float(result);
    }

    void setG(float _g) {
        if (_g >= 4) {
            g = 3.99999f;
        }
        g = _g;
    }

    void onSampleRateChange(float rate, float sampleTime)
    {
        const float cutoffHz = 20.f;
        float fcNormalized = cutoffHz * sampleTime;
        assert((fcNormalized > 0) && (fcNormalized < .1));

        ButterworthFilterDesigner<double>::designFourPoleHighpass(dcBlockParams, fcNormalized);
        configured = true;
    }

private:
    float x = .5f;
    float g = 3.9f; 
    int ct = 0;

    BiquadParams<double, 2> dcBlockParams;
    BiquadState<double, 2> dcBlockState;
    bool configured = false;

};

class SimpleChaoticNoise
{
public:
    float step() {
        return generator.step();
    }
    void setG(float g) {
        generator.setG(g);
    }
    void onSampleRateChange(float rate, float time) {
        generator.onSampleRateChange(rate, time);
    }
private:
    ChaosGen1 generator;
};


class ResonatorWithFilters : public RecirculatingFractionalDelay
{
public:
    ResonatorWithFilters(int maxSamples) : RecirculatingFractionalDelay(maxSamples) {
    }

    float processFeedback(float input) override {
        //  input = LowpassFilter<float>::run(input, lpfState, lpfParams);
        return std::min(10.f, std::max(-10.f, input));
        // return input;
        //return LowpassFilter<float>::run(input, lpfState, lpfParams);
    }

    /**
     * brightness = 0..1
     * resonance = 0..1
     * 
     * returns the makeup gain
     */
    float set(float freqHz, float sampleRate, float brightness, float resonance) {
        const float delaySeconds = 1.0f / freqHz;
        float delaySamples = delaySeconds * sampleRate;
        delaySamples = std::min(int(delaySamples), numSamples-1);
        setDelay(delaySamples);
      
        float cutoff = float(brightnessFunc(brightness) * freqHz / sampleRate);
        cutoff = std::min(cutoff, .4f);
        LowpassFilter<float>::setCutoff(lpfParams, cutoff);

        float reso = float(resonanceFunc(resonance)); 
        setFeedback(reso);
      //  gain = 1 / (1 - reso);
      return .1;
    }
private:
    LowpassFilterParams<float> lpfParams;
    LowpassFilterState<float> lpfState;

    // brighness is the Fc multiplier
    std::function<double(double)> brightnessFunc =  AudioMath::makeFunc_Exp(0, 1, 1, 20);
    std::function<double(double)> resonanceFunc =  AudioMath::makeFunc_Exp(0, 1, .9, .9999);

};

class ResonantNoise
{
public:
    ResonantNoise() : delay(2000) {
        delay.setDelay(200);
        delay.setFeedback(-.99f);
    }

    void onSampleRateChange(float rate, float time) {
        chaos.onSampleRateChange(rate, time);
        sampleRate = rate;
        sampleTime = time;
    }

    /**
     * brightness = 0..1
     * resonance = 0..1
     */
    void set(float freqHz, float brightness, float resonance) {
        makeupGain = delay.set(freqHz, sampleRate, brightness, resonance);
    }
   
    void setG(float g) {
        chaos.setG(g);
    }

    float step() {
        const float x = chaos.step();

        float x2 = delay.run(x);
        return float(2 * makeupGain * (x2));
        //return float(2 * makeupGain * (x2 - .5));
    }
    
private:
    float makeupGain = 1;
    ChaosGen1 chaos;
    ResonatorWithFilters delay;
    float sampleRate = 44100;
    float sampleTime = 1.f / 44100;
};
