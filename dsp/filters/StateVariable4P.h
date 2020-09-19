#pragma once

#include <algorithm>

template <typename T>
class StateVariableFilterParams4P
{
public:
    // for now let's pull out sample time
 //   void setFreqVolts(float volts, float sampleTime);
    void setFreq(float fcNormalized);
    void setNotch(bool);
  
    T fcg = T(-.1);
    T Rg = 3;
    T Qg = T(1.9);
    bool notch = false;
};


template <typename T>
inline void StateVariableFilterParams4P<T>::setNotch(bool enable)
{
    notch = enable;
}

template <typename T>
inline void StateVariableFilterParams4P<T>::setFreq(float normFc)
{
    normFc = std::max(normFc, .3f);
    fcg = -normFc;
}

#if 0
template <typename T>
inline void StateVariableFilterParams4P<T>::setFreqVolts(float volts, float sampleTime)
{
    // there is a a bug, this function doesn't work except simd
    // float freq = rack::dsp::FREQ_C4 * rack::dsp::approxExp2_taylor5(volts + 30) / 1073741824;
    float freq = rack::dsp::FREQ_C4 * std::exp2(volts + 30) / 1073741824;
    
    float g = -freq * sampleTime;
    // printf("volts = %f freq = %f g = %f\n", volts, freq, g); fflush(stdout);
    fcg = g;
}
#endif


template <typename T>
class StateVariableFilterState4P
{
public:
    T z1 = 0;		// the delay line buffer
    T z2 = 0;
    T z3 = 0;
    T z4 = 0;
    //
    T lp = 0;
    T hp = 0;
    T bp = 0;
    T peak = 0;
};


template <typename T>
class StateVariableFilter4P
{
public:
    StateVariableFilter4P() = delete;       // we are only static
    static void run(T input, StateVariableFilterState4P<T>& state, const StateVariableFilterParams4P<T>& params);
private:
 //   static T x(float input, float fcG, float& z);
    static void xx(float& delayMemory, float input, float fcG);
};

#if 0 // original way
    // maybe we try this again later?
template <typename T>
inline void StateVariableFilter4P<T>::xx(float& delayMemory, float input, float fcG)
{
     assert(fcG < 0);
     assert(fcG > -1);
    float temp = fcG * input;
    temp += delayMemory;
    delayMemory = temp;
}


template <typename T>
inline T StateVariableFilter4P<T>::run(T input, StateVariableFilterState4P<T>& state, const StateVariableFilterParams4P<T>& params)
{
    assert(params.fcg < 0);

#if 1      // turn off the real stuff for easier debuggin
    const float v1 = - (input + state.z4 + params.Rg * state.z2 - params.Qg * (state.z1 + state.z3));
#elif 1
    // no R or Q feedback
    const float v1 = - (input + state.z4);
#else 
    const float v1 = -input;
#endif
    const float output = state.z4;        // extra sample delay? don't need to do this.


    xx(state.z4, state.z3, params.fcg);
    xx(state.z3, state.z2, params.fcg);
    xx(state.z2, state.z1, params.fcg);
    xx(state.z1, v1, params.fcg);

    return output;
}
#else

template <typename T>
inline void StateVariableFilter4P<T>::run(T input, StateVariableFilterState4P<T>& state, const StateVariableFilterParams4P<T>& params)
{
    const float v5 = state.z4;
    const float v4 = state.z3;
    const float v3 = state.z2;
    const float v2 = state.z1;

    state.bp =  (v2 + v4);
    const float rOut = params.Rg * v3;
    const float v0 = input + v5 + rOut - (params.Qg * state.bp);
    const float v1 = -v0;

    state.z4 = v4 * params.fcg + v5;
    state.z3 = v3 * params.fcg + v4;
    state.z2 = v2 * params.fcg + v3;
    state.z1 = v1 * params.fcg + v2;

    state.lp = v5;
    state.hp = v1;
    state.peak = v1 + state.lp + (params.notch ? rOut : 0);
}

#endif