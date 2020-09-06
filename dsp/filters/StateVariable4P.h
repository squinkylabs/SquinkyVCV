#pragma once

template <typename T>
class StateVariableFilterParams4P
{
public:
    T fcg = T(-.3);
    T Rg = 0;
    T Qg = 0;

  //    fcGain = T(AudioMath::Pi) * T(2) * fc;
};



template <typename T>
class StateVariableFilterState4P
{
public:
    T z1 = 0;		// the delay line buffer
    T z2 = 0;
    T z3 = 0;
    T z4 = 0;
};


template <typename T>
class StateVariableFilter4P
{
public:
    StateVariableFilter4P() = delete;       // we are only static
    static T run(T input, StateVariableFilterState4P<T>& state, const StateVariableFilterParams4P<T>& params);
private:
 //   static T x(float input, float fcG, float& z);
    static void xx(float& delayMemory, float input, float fcG);
};

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

#if 0       // turn off the real stuff for easier debuggin
    const float v1 = - (input + state.z4 + params.Rg * state.z2 - params.Qg * (state.z1 + state.z3));
#else
    const float v1 = - (input + state.z4);
#endif
    const float output = state.z4;        // extra sample delay? don't need to do this.


    xx(state.z4, state.z3, params.fcg);
    xx(state.z3, state.z2, params.fcg);
    xx(state.z2, state.z1, params.fcg);
    xx(state.z1, v1, params.fcg);

    return output;
}