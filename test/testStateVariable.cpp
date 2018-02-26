#include <assert.h>

#include "StateVariableFilter.h"

template <typename T>
static void test1()
{
    StateVariableFilterParams<T> params;
    StateVariableFilterState<T> state;
    params.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params.setFreq(T(.01));                   // TODO: make an even fraction
    StateVariableFilter<T>::run(0, state, params);

}


template <typename T>
static void test()
{
   // test1<T>();

}

void testStateVariable()
{
    test<float>();
    test<double>();
}