#pragma once


template <typename T>
class LadderFilter
{
public:
    void run(T);
    T getOutput();
    void setFc(T);
private:
};

template <typename T>
inline T LadderFilter<T>::getOutput()
{
    return 0;
}

template <typename T>
inline void LadderFilter<T>::run(T input)
{
}

template <typename T>
inline void LadderFilter<T>::setFc(T input)
{
}