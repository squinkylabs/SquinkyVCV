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

    assert(gl == 1);
}

/**
 * Measure freq response at some points
 */
template <typename T>
static void test3()
{
    const T fc = T(.001);// was .05
    const T q = T(1.0 /  std::sqrt(2));// butterworth
    StateVariableFilterParams<T> params;
    StateVariableFilterState<T> state;
    params.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params.setFreq(fc);                   // TODO: make an even fraction
    params.setQ(q);   // tried .7

    double g = measureGain<T>(fc/4, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g = AudioMath::db(g);
    assert(AudioMath::closeTo(g, 0, .05));

    g = measureGain<T>(fc, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g = AudioMath::db(g);
    assert(AudioMath::closeTo(g, -3, .05));

    double g2 = measureGain<T>(fc*4, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g2 = AudioMath::db(g2);

    double g3 = measureGain<T>(fc * 8, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g3 = AudioMath::db(g3);
    assert(AudioMath::closeTo(g2-g3, 12, 2));


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