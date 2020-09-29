#pragma once

#include "LowpassFilter.h"
#include <algorithm>

#define _HPF
template <typename T>
class StateVariableFilterParams4P
{
public:
    // for now let's pull out sample time
    void setFreq(float fcNormalized);
    void setNotch(bool);
    void setQ(float q);
    void setR(float r);

    /**
     * sampleTime is 1/sampleRate
     */
    void onSampleTimeChange(float st);
  
    T _fcGain() const { return fcg; }
    T _qGain() const { return Qg; }

    LowpassFilterParams<T> dcBlockParams;
    T fcg = T(-.1);
    T Rg = 3;
    T Qg = T(1.9);
    bool notch = false;
};

template <typename T>
inline void StateVariableFilterParams4P<T>::onSampleTimeChange(float st)
{
    float normFc = st * 2;      // 2 hz
    LowpassFilter<T>::setCutoff(dcBlockParams, normFc);
     printf("** onSampleTimeChange: hpParms l = %f k = %f\n", dcBlockParams.l, dcBlockParams.k);
}

template <typename T>
inline void StateVariableFilterParams4P<T>::setR(float r)
{
    // if (r < 2.2) printf("clipping low (%f) R\n", r);
  //  printf("setting r = %f\n", r);
    Rg = std::max(r, 2.2f);
}

template <typename T>
inline void StateVariableFilterParams4P<T>::setQ(float q)
{
    // if (q < .1) printf("clipping low (%f) Q\n", q);
    Qg = std::max(q, .1f);
}

template <typename T>
inline void StateVariableFilterParams4P<T>::setNotch(bool enable)
{
    notch = enable;
}

template <typename T>
inline void StateVariableFilterParams4P<T>::setFreq(float normFc)
{
   // normFc = std::max(normFc, .3f);
 //   printf("after max, norm = %f\n", normFc);
    
    fcg = -normFc *T(AudioMath::Pi) * T(2) ;
  //  printf("sv4p set fcg to %f\n", fcg);
}

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

#ifdef _HPF
    LowpassFilterState<T> dcBlockState;
#endif
};


template <typename T>
class StateVariableFilter4P
{
public:
    StateVariableFilter4P() = delete;       // we are only static
    static void run(T input, StateVariableFilterState4P<T>& state, const StateVariableFilterParams4P<T>& params);
private:
    static void xx(float& delayMemory, float input, float fcG);
};

// third attempt
// so far, much better, but still very unstable.
// When R is 5.8, it blows up when high peak hits 13k
// when Ris 3 blows up when the single peak hits 1k.
// maybe R has DC?

#if 1
template <typename T>
inline void StateVariableFilter4P<T>::run(T input, StateVariableFilterState4P<T>& state, const StateVariableFilterParams4P<T>& params)
{
#if 1
    static float rOutMax = 0;
    static float rOutMin = 0;
    static float total = 0;
    static int samples=0;
#endif

    assert(params.fcg < 0);
    // z1 is special - it's a delay to make the difference eqn realizable
    const float v2 = state.z1;
    const float v3 = state.z2 + params.fcg * v2;
    const float v4 = state.z3 + params.fcg * v3;
    const float v5 = state.z4 + params.fcg * v4;

    const float rOutRaw = params.Rg * v3;
#ifdef _HPF
    const float lpRout =  LowpassFilter<T>::run(rOutRaw, state.dcBlockState, params.dcBlockParams); 
    const float rOut = rOutRaw - lpRout;
#else
    const float rOut = rOutRaw;
#endif
   
  //  const float rOut = LowpassFilter<T>::run(rOutRaw, state.hpState, params.hpParams);


   // printf("hpParms l = %f k = %f\n", params.hpParams.l, params.hpParams.k);
  //  printf("rOutRaw = %f, rOut = %f\n", rOutRaw, rOut); fflush(stdout);

#if 0
      assert(rOut < 10000);
      assert(rOut > - 10000);
       assert(rOutRaw < 10000);
      assert(rOutRaw > - 10000);
#endif

    const float v0 = input + v5 + rOut - (params.Qg * state.bp);
    const float v1 = -v0;

    state.bp =  (v2 + v4);
    state.lp = v5 + (params.notch ? v3 : 0);        // might need to scale v3
    state.hp = v1 + (params.notch ? v3 : 0); 
    state.peak = v1 + v5 + (params.notch ? rOut : 0);
#if 0

    {
        samples ++;
        total += rOut;
        float dc = total / samples;

        if (rOut > rOutMax) {
            rOutMax = rOut;
            printf("new RMAX = %.2f rmin-%.2f dc=%f, ct=%d\n", rOutMax, rOutMin, dc, samples);
        }
        if (rOut < rOutMin) {
            rOutMin = rOut;
            printf("new RMin = %.2f max is %.2f dc=%f ct=%d\n", rOutMin, rOutMax, dc, samples);
        }
        fflush(stdout);
      //  assert(dc < 2);
        // assert(dc > -2);
      

        //   if (rOut < rOutMin) rOutMin = rOut;
        // printf("RR - %f - %f v3=%f\n", rOutMin, rOutMax, v3);
        

        //  assert(rOut < 10000);
        //  assert(rOut > - 10000);

       // printf("state [%f, %f, %f, %f] v3=%f rOut=%f\n", state.z1, state.z2, state.z3, state.z4, v3, rOut);
    }
    #endif


    // can we move these to the end?
    state.z4 = v5;
    state.z3 = v4;
    state.z2 = v3;
    state.z1 = (v1 * params.fcg) + v2;
}
#endif

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
#endif

#if 0      // second way. works but unstable
template <typename T>
inline void StateVariableFilter4P<T>::run(T input, StateVariableFilterState4P<T>& state, const StateVariableFilterParams4P<T>& params)
{
    static float rOutMax = 0;
    static float rOutMin = 0;

    const float v5 = state.z4;
    const float v4 = state.z3;
    const float v3 = state.z2;
    const float v2 = state.z1;

    
    

    const float rOut = params.Rg * v3;
    const float v0 = input + v5 + rOut - (params.Qg * state.bp);
    const float v1 = -v0;

    {
        if (rOut > rOutMax) rOutMax = rOut;
        if (rOut < rOutMin) rOutMin = rOut;
       // printf("RR - %f - %f v3=%f\n", rOutMin, rOutMax, v3);

      //  assert(rOut < 10000);
      //  assert(rOut > - 10000);

        printf("state [%f, %f, %f, %f] v3=%f rOut=%f\n", state.z1, state.z2, state.z3, state.z4, v3, rOut);
    }

    // can we move these to the end?
    state.z4 = v4 * params.fcg + v5;
    state.z3 = v3 * params.fcg + v4;
    state.z2 = v2 * params.fcg + v3;
    state.z1 = v1 * params.fcg + v2;

    state.lp = v5 + (params.notch ? v3 : 0);        // might need to scale v3

    state.hp = v1 + (params.notch ? v3 : 0); 
    state.peak = v1 + v5 + (params.notch ? rOut : 0);
}

#endif