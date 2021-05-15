#pragma once

#include "SqLog.h"
#include <assert.h>
#include <map>

template <typename T> class NonUniformLookupTable;

template <typename T>
class NonUniformLookupTableParams
{
public:
    friend NonUniformLookupTable<T>;
    NonUniformLookupTableParams() = default;
    NonUniformLookupTableParams(const NonUniformLookupTableParams&) = delete;
    NonUniformLookupTableParams& operator= (const NonUniformLookupTableParams&) = delete;
    int size() const
    {
        return (int) entries.size();
    }
    void _dump() {
        SQINFO("dump table");
        SQINFO(" there are %d entries", entries.size());
        for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
            SQINFO("  entr x=%f, y=%f, a=%f", iter->second.x, iter->second.y, iter->second.a);
            auto iter2 = iter;
            ++iter2;
            if (iter2 != entries.end()) {
                SQINFO("dy/dx = %f", (iter2->second.y - iter->second.y) / (iter2->second.x - iter->second.x));
               
            }
        }
#if 0
        for (auto entry : entries) {
            SQINFO("  entr x=%f, y=%f, a=%f", entry.second.x, entry.second.y, entry.second.a);
        }
#endif
        SQINFO("done");
    }
private:

    class Entry
    {
    public:
        T x;
        T y;
        T a;
    };
    using container = std::map<T, Entry>;
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
    static T lookup(const NonUniformLookupTableParams<T>& params, T x);
};

template <typename T>
inline void NonUniformLookupTable<T>::addPoint(NonUniformLookupTableParams<T>& params, T x, T y)
{
    using Entry = typename NonUniformLookupTableParams<T>::Entry;
    Entry e;
    e.x = x;
    e.y = y;
    params.entries.insert(std::pair<T, Entry>(x, e));
}

template <typename T>
inline void NonUniformLookupTable<T>::finalize(NonUniformLookupTableParams<T>& params)
{
    assert(!params.isFinalized);

    using iterator = typename std::map<T, typename NonUniformLookupTableParams<T>::Entry>::iterator;
    iterator it;

    for (it = params.entries.begin(); it != params.entries.end(); ++it) {
        iterator it_next = it;
        ++it_next;

        // Will now generate a line segment from this entry to the next
        if (it_next == params.entries.end()) {
            it->second.a = 0;
        } else {
            T a = (it_next->second.y - it->second.y) / (it_next->second.x - it->second.x);
            it->second.a = a;
        }
    }

    params.isFinalized = true;
}


template <typename T>
inline T NonUniformLookupTable<T>::lookup(const NonUniformLookupTableParams<T>& params, T x)
{
    assert(params.isFinalized);
    assert(!params.entries.empty());

    auto lb_init = params.entries.lower_bound(x);
    auto lb = lb_init;

    if (lb == params.entries.end()) {
        return params.entries.rbegin()->second.y;
    }
    if (x >= lb->second.x) {
        // this could only happen if we hit equal
    } else {
        // added this case to keep mac from crashing, as we were trying to decrement begin()
        if (lb == params.entries.begin()) {
            return lb_init->second.y;
        }
        --lb;
        if (lb == params.entries.end()) {
            // I thing now that above is fixed this won't happen?
            assert(false);
            return lb_init->second.y;
        }
    }

    // Now that we have the right entry, interpolate.
    T ret = lb->second.a * (x - lb->second.x) + lb->second.y;
    return ret;
}

