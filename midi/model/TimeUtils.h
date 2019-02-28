#pragma once

#include <tuple>

class TimeUtils
{
public:
    static float barToTime(int bar)
    {
        return bar * 4.f;     // for now, 4 q in one bar
    }

    static int timeToBar(float time)
    {
        return (int) std::floor(time / 4.f);
    }

    static float quarterNote()
    {
        return 1;
    }

    static std::tuple<int, int, int> time2bbf(float time)
    {
        const int bar = timeToBar(time);
        float remaining = time - barToTime(bar);
        const int q = (int) std::floor(remaining);
        remaining -= q;
        const int f = int( std::round(remaining * 100));
        return std::make_tuple(bar, q, f);
    }

    static std::string time2str(float time)
    {
        auto bbf = time2bbf(time);
        char buffer[256];
        sprintf_s(buffer, "%d.%d.%d", std::get<0>(bbf), std::get<1>(bbf), std::get<2>(bbf));
        return buffer;
    }
};