#pragma once

#include <atomic>
class NoiseSharedState
{
public:
    NoiseSharedState()
    {
        ++_dbgCount;
    }
    ~NoiseSharedState()
    {
        --_dbgCount;
    }

    static std::atomic<int> _dbgCount;
};