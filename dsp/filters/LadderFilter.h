#pragma once

#include "LookupTable.h"
#include "TrapezoidalLowpass.h"

template <typename T>
class LadderFilter
{
public:
    void run(T);
    T getOutput();

    /**
     * input range >0 to < .5
     */
    void setNormalizedFc(T);

    void setFeedback(T f);
    T getRawOutput(int stage);
private:
    TrapezoidalLowpass<T> lpfs[4];
    T _g = .001f;
    T output = 0;
    T feedback = 0;
    T rawOutput[4];

    std::shared_ptr<NonUniformLookupTableParams<T>> fs2gLookup = makeTrapFilter_Lookup<T>();
};

template <typename T>
inline T LadderFilter<T>::getOutput() 
{
    return output;
}

template <typename T>
inline T LadderFilter<T>::getRawOutput(int i)
{
    assert(i < 4);
    assert(i >= 0);
    return rawOutput[i];
}

template <typename T>
inline void LadderFilter<T>::setFeedback(T f)
{
    feedback = f;
}

template <typename T>
inline void LadderFilter<T>::run(T input)
{
    input = input - feedback * output;
    T temp = lpfs[0].run(input, _g);
    rawOutput[0] = temp;
    temp = lpfs[1].run(temp, _g);
    rawOutput[1] = temp;
    temp = lpfs[2].run(temp, _g);
    rawOutput[2] = temp;
    temp = lpfs[3].run(temp, _g);
    rawOutput[3] = temp;
    output = temp;
}

template <typename T>
inline void LadderFilter<T>::setNormalizedFc(T input)
{
    const T g2 = NonUniformLookupTable<T>::lookup(*fs2gLookup, input);
    _g = g2;
}