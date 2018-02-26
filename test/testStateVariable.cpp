#include <assert.h>

#include "StateVariableFilter.h"
#include "TestSignal.h"

template <typename T>
T measureGain(T freq, std::function<T(T)> func)
{
    const int size = 60000;
    T input[size];
    T output[size];

    //generateSin(T * output, int numSamples, T freq);
    TestSignal<T>::generateSin(input, size, freq);
    for (int i = 0; i < size; ++i) {
        output[i] = func(input[i]);
    }
    T vo =  T(TestSignal<T>::getRMS(output, size));
    T vi = T(TestSignal<T>::getRMS(input, size));
    return vo / vi;
}


/**
 * Simple test - can we get output that looks kind of low pass
 */
template <typename T>
static void test1()
{
    StateVariableFilterParams<T> params;
    StateVariableFilterState<T> state;
    params.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params.setFreq(T(.05));                   // TODO: make an even fraction
    params.setQ(T(.7));

    T lastValue = -1;
    for (int i = 0; i < 5; ++i) {
        T output = StateVariableFilter<T>::run(1, state, params);
        assert(output > lastValue);
        lastValue = output;
    }
}

// move this to testTestSignal
template <typename T>
static void test2()
{
    /**
    StateVariableFilterParams<T> params;
    StateVariableFilterState<T> state;
    params.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params.setFreq(T(.05));                   // TODO: make an even fraction
    params.setQ(T(.7));
    */

    const T gl = measureGain<T>(T(.001), [](T input) {
        return input;
        });

    printf("gl = %f\n", gl);
    assert(gl == 1);
   

}

template <typename T>
static void test3()
{
  
    StateVariableFilterParams<T> params;
    StateVariableFilterState<T> state;
    params.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params.setFreq(T(.05));                   // TODO: make an even fraction
    params.setQ(T(.7));


    const T gl = measureGain<T>(T(.001), [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });

    printf("gl = %f\n", gl);
    assert(gl == 1);


}

template <typename T>
static void test()
{
    test1<T>();
    test2<T>();
    test3<T>();
}

void testStateVariable()
{
    test<float>();
    test<double>();
}