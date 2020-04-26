#pragma once

#include "FractionalDelay.h"
#include "LowpassFilter.h"

class SimpleChaoticNoise
{
public:
    float step() {
        const float next = g * x * (1 - x);
        x = next;
      //  printf("%f\n", x);fflush(stdout);
        return float(5 * (next - .5));
    }
    void setG(float _g) {
        if (_g >= 4) {
            g = 3.99999f;
        }
        g = _g;
    }

    float _getG() const {
        return g;
    }
private:
    float x = .5f;
    float g = 3.9f; 
};

class ResonatorWithFilters : public RecirculatingFractionalDelay
{
public:
    ResonatorWithFilters(int maxSamples) : RecirculatingFractionalDelay(maxSamples) {
    }
    float processFeedback(float input) override {
        return LowpassFilter<float>::run(input, lpfState, lpfParams);
    }
#if 0
    void setFreqHz(float freq, float sampleRate) {
        const float delaySeconds = 1.0f / freq;
        float delaySamples = delaySeconds * sampleRate;
        setDelay(delaySamples);
        // printf("set cutoff %f\n", freq / sampleRate);
        LowpassFilter<float>::setCutoff(lpfParams, 6 * freq / sampleRate);
    }
    #endif

    /**
     * brightness = 0..1
     * resonance = 0..1
     */
    void set(float freqHz, float sampleRate, float brightness, float resonance) {
        const float delaySeconds = 1.0f / freqHz;
        float delaySamples = delaySeconds * sampleRate;
        setDelay(delaySamples);
      
        float cutoff = brightnessFunc(brightness) * freqHz / sampleRate;
        cutoff = std::min(cutoff, .4f);
        LowpassFilter<float>::setCutoff(lpfParams, cutoff);

        // printf("set cutoff %f\n", cutoff);

        float reso = resonanceFunc(resonance); 
        setFeedback(-reso);
        // printf("set fc %f, reo %f\n", cutoff, reso); fflush(stdout);
    }
private:
    LowpassFilterParams<float> lpfParams;
    LowpassFilterState<float> lpfState;

    // AudioMath::makeFunc_Exp(double xMin, double xMax, double yMin, double yMax);
    // brighness is the Fc multiplier
    std::function<double(double)> brightnessFunc =  AudioMath::makeFunc_Exp(0, 1, 1, 20);
    std::function<double(double)> resonanceFunc =  AudioMath::makeFunc_Exp(0, 1, .9, .999);

};

class ResonantNoise
{
public:
    ResonantNoise() : delay(2000) {
        delay.setDelay(200);
        delay.setFeedback(-.99f);
    }

    /**
     * brightness = 0..1
     * resonance = 0..1
     */
    void set(float freqHz, float sampleRate, float brightness, float resonance) {
        delay.set(freqHz, sampleRate, brightness, resonance);
    }
   

    float step() {
        // first, do feedback chaos gen
        const float next = g * x * (1 - x);
        x = next;

        float x2 = delay.run(x);
        return float(2 * (x2 - .5));
    }
    void setG(float _g) {
        if (_g >= 4) {
            g = 3.99999f;
        }
        g = _g;
    }

    float _getG() const {
        return g;
    }
private:
    float x = .5f;
    float g = 3.9f;
    int ct = 0;

    ResonatorWithFilters delay;
};

//************************** failures *******************

#if 0


class ResonantNoiseNG
{
public:
    ResonantNoise() : delay(2000) {
        delay.setDelay(300);
    }
    float step() {
        float delayOut = delay.run(x);
        if (delayOut == 0) {
            delayOut = .5;
        }
        x = g * delayOut * (1 - delayOut);

        bool print = false;
#if 1
        if (ct < 10000) {
            if (0 == (ct % 100)) {
                print = true;
                printf("k3 iter %d, delay = %.2f, x = %.2f g = %.2f\n", ct, delayOut, x, g);
            }

        }
        ++ct;
#endif

        x -= (float)(delayOut * .9);    // let's recirc a little
        if (print) {
            printf("x with fedbck = %.2f\n", x);
        }
        return x;
    }
    void setG(float _g) {
        if (_g >= 4) {
            g = 3.99999f;
        }
        g = _g;
    }
private:
    float x = .5f;
    float g = 3.9f;
    int ct = 0;

    FractionalDelay delay;
};








class Kitty2
{
public:
    float step() {
        double xNext = std::sin((a + delta) * y) - std::cos(b * x);
        double yNext = std::sin(c * x) - std::cos(d * y);

        x = xNext;
        y = yNext;

        float ret = float((y == 0) ? 1 : (x / y));
        return ret;
    }

    void setDelta(float d) {
        delta = d;
    }
private:
    double x = 1;
    double y = 1;

    double delta = 0;
#if 1
    double a = 1.641;
    double b = 1.902;
    double c = .316;
    double d = 1.525;
#endif

#if 0
    double a = -0.827;
    double b = -1.637;
    double c = 1.659;
    double d = -0.943;
#endif

#if 0
    double a = -2.24;
    double b = 0.43;
    double c = -0.65;
    double d = -2.43;
#endif
};


class Kitty4
{
public:
    Kitty4() : delayX(2000), delayY(2000)
    {
        delayX.setDelay(500);
        delayY.setDelay(500);
    }
    float step() {
        double delayOutX = delayX.run(float(x));
        double delayOutY = delayY.run(float(y));
        x  = std::sin((a + delta) * delayOutY) - std::cos(b * delayOutX);
        y = std::sin(c * delayOutX) - std::cos(d * delayOutY);

        return float(x);
    }

    void setDelta(float d) {
        delta = d;
    }
private:
    double x = 1;
    double y = 1;
    FractionalDelay delayX;
    FractionalDelay delayY;

    double delta = 0;
#if 1
    double a = 1.641;
    double b = 1.902;
    double c = .316;
    double d = 1.525;
#endif

#if 0
    double a = -0.827;
    double b = -1.637;
    double c = 1.659;
    double d = -0.943;
#endif

#if 0
    double a = -2.24;
    double b = 0.43;
    double c = -0.65;
    double d = -2.43;
#endif
};

#endif

