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
        T a;
    };
    using container = std::map<T, Entry>;
  //  using iterator = container::iterator;
  //  typedef container::iterator iterator;
   // using iterator = std::map<T, NonUniformLookupTableParams::Entry>::iterator;
    bool isFinalized = false;
    container entries;
};

template <typename T>
class NonUniformLookupTable
{
public:
    NonUniformLookupTable() = delete;
    static void addPoint(NonUniformLookupTableParams<T>& params, T x, T y);
    static void finalize(NonUniformLookupTableParams<T>& params);
    static T lookup(NonUniformLookupTableParams<T>& params, T x);
};

template <typename T>
inline void NonUniformLookupTable<T>::addPoint(NonUniformLookupTableParams<T>& params, T x, T y)
{
    typename NonUniformLookupTableParams<T>::Entry e;
    e.x = x;
    e.y = y;
    params.entries.insert(std::pair<T, typename NonUniformLookupTableParams<T>::Entry>(x, e));
  //  params.entries.insert({x, e});
}

template <typename T>
inline void NonUniformLookupTable<T>::finalize(NonUniformLookupTableParams<T>& params)
{
    assert(!params.isFinalized);

    typename std::map<T, typename NonUniformLookupTableParams<T>::Entry>::iterator it;
    for (it = params.entries.begin(); it != params.entries.end(); ++it) {
        typename std::map<T, typename NonUniformLookupTableParams<T>::Entry>::iterator it_next = it;
        ++it_next;

        // Will now generate a line segment from this entry to the next
        if (it_next == params.entries.end()) {
            //printf("in last segment, will set a == 0");
            it->second.a = 0;
        } else {
            T a = (it_next->second.y - it->second.y) / (it_next->second.x - it->second.x);
            //printf("for segment will use a = %f and b = %f\n", a, it->second.y);
            it->second.a = a;
        }
    }

    params.isFinalized = true;
}


template <typename T>
inline T NonUniformLookupTable<T>::lookup(NonUniformLookupTableParams<T>& params, T x)
{
    //printf("lookup x=%f\n", x);
    assert(params.isFinalized);
    assert(!params.entries.empty());

    auto lb_init = params.entries.lower_bound(x);
    auto lb = lb_init;

    if (lb == params.entries.end()) {
        return params.entries.rbegin()->second.y;
    }
   // assert(lb);
   // printf("lower bound x=%f y=%f\n", lb->second.x, lb->second.y);
    if (x >= lb->second.x) {
        // this could only happen if we hit equal
        //printf("Lb is in case 1\n");
    } else {
        --lb;
        if (lb == params.entries.end()) {
           // printf("needed prev, bit didn't find");
            return lb_init->second.y;
        }
    }
   // printf("after adjust, lb x=%f\n", lb->second.x);

    T ret = lb->second.a * (x - lb->second.x) + lb->second.y;
    //printf("not doint interp\n");
   // T ret = lb->second.y;           // no interpolation
   
    return ret;
}

