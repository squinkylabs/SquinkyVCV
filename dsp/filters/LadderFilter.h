#pragma once

#include "LookupTable.h"
#include "ObjectCache.h"
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
    std::shared_ptr<LookupTableParams<float>> tanhLookup = ObjectCache<float>::getTanh5();
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
    const float k = 1.f / 5.f;
    const float j = 1.f / k;

    input = input - feedback * output;
    input = j * LookupTable<float>::lookup(*tanhLookup.get(), k * input, true);
    T temp = lpfs[0].run(input, _g);

    temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    rawOutput[0] = temp;
    temp = lpfs[1].run(temp, _g);

    temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    rawOutput[1] = temp;
    temp = lpfs[2].run(temp, _g);

    temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    rawOutput[2] = temp;
    temp = lpfs[3].run(temp, _g);

    rawOutput[3] = temp;

    temp = std::max(T(-10), temp);
    temp = std::min(T(10), temp);
    output = temp;
}

template <typename T>
inline void LadderFilter<T>::setNormalizedFc(T input)
{
    const T g2 = NonUniformLookupTable<T>::lookup(*fs2gLookup, input);
    _g = g2;
}