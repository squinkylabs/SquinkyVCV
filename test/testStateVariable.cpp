#include <assert.h>

#include "StateVariableFilter.h"
#include "TestSignal.h"

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


/**
 * Measure freq response at some points
 */
template <typename T>
static void testLowpass()
{
    const T fc = T(.001);// was .05
    const T q = T(1.0 / std::sqrt(2));// butterworth
    StateVariableFilterParams<T> params;
    StateVariableFilterState<T> state;
    params.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params.setFreq(fc);                   // TODO: make an even fraction
    params.setQ(q);   // tried .7

    double g = TestSignal<T>::measureGain(fc / 4, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g = AudioMath::db(g);
    assert(AudioMath::closeTo(g, 0, .05));

    g = TestSignal<T>::measureGain(fc, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g = AudioMath::db(g);
    assert(AudioMath::closeTo(g, -3, .05));

    double g2 = TestSignal<T>::measureGain(fc * 4, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g2 = AudioMath::db(g2);

    double g3 = TestSignal<T>::measureGain(fc * 8, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g3 = AudioMath::db(g3);
    assert(AudioMath::closeTo(g2 - g3, 12, 2));
}

template <typename T>
static void test()
{
    test1<T>();
    testLowpass<T>();
}

void testStateVariable()
{
    test<float>();
    test<double>();
}