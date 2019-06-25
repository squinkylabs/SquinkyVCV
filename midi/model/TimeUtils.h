#pragma once

#include <assert.h>
#include <stdio.h>
#include <tuple>

class TimeUtils
{
public:
    static float bar2time(int bar)
    {
        return bar * 4.f;     // for now, 4 q in one bar
    }

    static int time2bar(float time)
    {
        return (int) std::floor(time / 4.f);
    }

    static float quarterNote()
    {
        return 1;
    }

    static std::tuple<int, int, int> time2bbf(float time)
    {
        const int bar = time2bar(time);
        float remaining = time - bar2time(bar);
        const int q = (int) std::floor(remaining);
        remaining -= q;
        const int f = int( std::round(remaining * 100));
        return std::make_tuple(bar, q, f);
    }

    /**
     * Quantizes a midi time into a number of bars and a remaining time
     * number of bars will be quantized to numBarsInUnit.
     */
    static std::tuple<int, float> time2barsAndRemainder(int numBarsInUnit, float time)
    {
        int barsN = int(time / bar2time(numBarsInUnit));
        float t0 = barsN * TimeUtils::bar2time(numBarsInUnit);
        float remainder = time - t0;
        assert(remainder >= 0);
        return std::make_tuple(barsN, remainder);
    }

    /**
     * Converts a regular metric time to bar:beat:hundredths.
     * bars and beats start numbering at 1, like humans do.
     */
    static std::string time2str(float time)
    {
        auto bbf = time2bbf(time);
        char buffer[256];
        snprintf(buffer, 256, "%d.%d.%d", 1 + std::get<0>(bbf), 1 + std::get<1>(bbf), std::get<2>(bbf));
        return buffer;
    }
};