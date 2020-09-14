#pragma once

#include "simd.h"

/**
 * MultiLag2 is based on MultiLag, but uses VCV SIMD library
 */


class MultiLPF2 {
public:
    float_4 get() const;
};

inline float_4 MultiLPF2::get() const
{
    return float_4(0);
}


class MultiLag2 {
public:
    float_4 get() const;
};

inline float_4 MultiLag2::get() const
{
    return float_4(0);
}