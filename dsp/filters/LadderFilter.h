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
private:
    TrapezoidalLowpass<T> lpfs[4];
    T _g = .001f;
    T output = 0;

    std::shared_ptr<NonUniformLookupTableParams<T>> fs2gLookup = makeTrapFilter_Lookup<T>();
};

template <typename T>
inline T LadderFilter<T>::getOutput() 
{
    return output;
}

template <typename T>
inline void LadderFilter<T>::run(T input)
{
    T temp = lpfs[0].run(input, _g);
    temp = lpfs[1].run(temp, _g);
    temp = lpfs[2].run(temp, _g);
    temp = lpfs[3].run(temp, _g);
    output = temp;
}

template <typename T>
inline void LadderFilter<T>::setNormalizedFc(T input)
{
    const T g2 = NonUniformLookupTable<T>::lookup(*fs2gLookup, input);
    _g = g2;
}