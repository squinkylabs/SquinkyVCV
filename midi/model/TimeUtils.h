#pragma once

#include <assert.h>
#include <stdio.h>
#include <tuple>

class TimeUtils
{
public:
    /**
     * @returns and even multiple of quantize.
     * rounds to closest.
     */
    static double quantize(double value, double quantize, bool allowZero)
    {
        double div = (value + quantize / 2.0) / quantize;
        double x = floor(div);
        double ret =  x * quantize;
        if (!allowZero && (ret < quantize)) {
            assert(value >= 0);     // can't handle negative numbers
            ret = quantize;
        }
        return ret;
    }
    
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
        #if 0
        auto bbf = time2bbf(time);
        char buffer[256];
        snprintf(buffer, 256, "%d.%d.%d", 1 + std::get<0>(bbf), 1 + std::get<1>(bbf), std::get<2>(bbf));
        return buffer;
        #endif
        return time2str(time, 3);
    }

    static std::string time2str(float time, int digits)
    {
        auto bbf = time2bbf(time);
        char buffer[256];
        switch (digits) {
            case 3:
                snprintf(buffer, 256, "%d.%d.%d", 1 + std::get<0>(bbf), 1 + std::get<1>(bbf), std::get<2>(bbf));
                break;
            case 2:
                snprintf(buffer, 256, "%d.%d", 1 + std::get<0>(bbf), 1 + std::get<1>(bbf));
                break;
            case 1:
                snprintf(buffer, 256, "%d", 1 + std::get<0>(bbf));
                break;
            default:
                assert(false);
        }
        return buffer;
    }
    
    /**
     * Quantizes time, rounding a way a grid constrained editor should.
     * @param time is the time to quantize.
     * @param deltaTime is how much we want to move the un-quantized time.
     * @param units is the time unit we want to quantize to.
     * @returns quantize(time + deltaTime), such that we never quantize "backwards"
     */
    static float quantizeForEdit(float time, float deltaTime, float units)
    {
        assert(units > 0);
        float q = units * std::round((time + deltaTime) / units);
        if (deltaTime > 0) {
            if (q < time) {
                q += units;
            }
        } else {
            if (q > time) {
                q -= units;
            }
        }
        return q;
    }
};

