#pragma once

#include <assert.h>

#include <map>

#include "AudioMath.h"

template <typename Tkey>
class RandomRange {
public:
    RandomRange(Tkey lowVAlue);

    /**
     * must be added with key increasing
     */
    void addRange(Tkey);

    int _lookup(Tkey);

    /**
     * generate a new random int
     */
    int get();

private:
    std::map<Tkey, int> theMap;
    int nextIndex = 0;
    Tkey lastKey = Tkey(-100);
    AudioMath::RandomUniformFunc rand = AudioMath::random();
};

template <typename Tkey>
inline int RandomRange<Tkey>::get() {
    auto r = rand();
   // printf("generated random f %f\n", r);
    return _lookup(r);
}

template <typename Tkey>
inline void RandomRange<Tkey>::addRange(Tkey key) {
   // printf("add range %f\n", key);
    theMap.insert({key, nextIndex});
    ++nextIndex;
    assert(key >= lastKey);
    lastKey = key;
#if 0
    printf("after add to range\n");
    for (auto it : theMap) {
        printf("  entry %f, %d\n", it.first, it.second);
    }
#endif
}

template <typename Tkey>
inline RandomRange<Tkey>::RandomRange(Tkey lowValue) {
    addRange(lowValue);  // start first range as lowValue
}

template <typename Tkey>
inline int RandomRange<Tkey>::_lookup(Tkey key) {
    auto it = theMap.lower_bound(key);
    int ret = 0;

    // printf("map size = %d\n", int(theMap.size()));
    if (it == theMap.end()) {
        if (theMap.empty()) {
            assert(false);
            return 0;
        }

        it--;
        // printf("in mapVelToPlayer vel=%d, went off end of map prev index=%d\n", vel, it->first); fflush(stdout);
        return it->second;
    }
    Tkey lb_key = it->first;
    if (lb_key > key) {
        --it;
        ret = it->second;
    } else if (lb_key == key) {
        ret = it->second;
    } else {
        // printf("in mapVelToPlayer vel=%d, lb_key =%d\n", vel, lb_key); fflush(stdout);
        assert(false);
    }
    return ret;
}