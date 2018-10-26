#pragma once


template <typename T>
class NonUniformLookupTableParams
{
public:
private:
    class Entry
    {
    public:
        T x;
        T y;
    };
};

template <typename T>
class NonUniformLookupTable
{
public:
    static void addPoint(NonUniformLookupTableParams<T>& params, T x, T y);
    static void finalize(NonUniformLookupTableParams<T>& params);
    static T lookup(NonUniformLookupTableParams<T>& params, T x);
};

template <typename T>
inline void NonUniformLookupTable<T>::addPoint(NonUniformLookupTableParams<T>& params, T x, T y)
{

}

template <typename T>
inline void NonUniformLookupTable<T>::finalize(NonUniformLookupTableParams<T>& params)
{

}


template <typename T>
inline T NonUniformLookupTable<T>::lookup(NonUniformLookupTableParams<T>& params, T x)
{
    return 0;
}

