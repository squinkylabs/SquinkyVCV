#pragma once

#include <assert.h>
#include <map>

template <typename T> class NonUniformLookupTable;

template <typename T>
class NonUniformLookupTableParams
{
public:
    friend NonUniformLookupTable<T>;
private:
    class Entry
    {
    public:
        T x;
        T y;
    };
    bool isFinalized = false;
    std::map<T, Entry> entries;
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
    NonUniformLookupTableParams<T>::Entry e;
    e.x = x;
    e.y = y;
    params.entries.insert({x, e});
}

template <typename T>
inline void NonUniformLookupTable<T>::finalize(NonUniformLookupTableParams<T>& params)
{
    assert(!params.isFinalized);
    params.isFinalized = true;
}


template <typename T>
inline T NonUniformLookupTable<T>::lookup(NonUniformLookupTableParams<T>& params, T x)
{
    assert(params.isFinalized);
    assert(!params.entries.empty());

    auto lb = params.entries.lower_bound(x);
   // assert(lb);
    printf("lower bound x=%f\n", lb->first);
    T ret = lb->second.y;           // no interpolation
    return ret;
}

