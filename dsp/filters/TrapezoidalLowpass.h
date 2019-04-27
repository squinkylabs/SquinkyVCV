#pragma once


/**
 * a one pole lowpass filter
 * has 6db less control voltage feedthrough than standard lpf
 */
template <typename T>
class TrapezoidalLowpass
{
public:
    T run(T g2, T input);

#if 0
    void setG(T x)
    {
        _g = x;
        _g2 = _g / (1 + _g);
    }
#endif
    static T legacyCalcG2(T g);
private:
    T _z = 0;
  //  T _g = 0;
 //   T _g2 = 0;
};

template <typename T>
inline T TrapezoidalLowpass<T>::legacyCalcG2(T g)
{
    return g / (1 + g);
}

template <typename T>
inline T TrapezoidalLowpass<T>::run(T vin, T _g2)
{
    const T temp = (vin - _z) * _g2;
    const T output = temp + _z;
    _z = output + temp;
    return output;
}

