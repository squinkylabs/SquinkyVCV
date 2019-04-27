#pragma once

#include "TrapezoidalLowpass.h"

template <typename T>
class LadderFilter
{
public:
    void run(T);
    T getOutput();
    void setFc(T);
private:
    TrapezoidalLowpass<T> lpfs[4];
    T _g = .001f;
    T output = 0;
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
inline void LadderFilter<T>::setFc(T input)
{
}