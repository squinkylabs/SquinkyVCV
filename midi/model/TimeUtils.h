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

    static float sixtyFourthNote()
    {
        return 1.f / 16.f;  
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
        return time2str(time, 3);
    }

    static std::string length2str(float time)
    {
        return time2str(time, 3, false);
    }

    static std::string time2str(float time, int digits)
    {
        return time2str(time, digits, true);
    }

    static std::string time2str(float time, int digits, bool addOneForHumanReadability)
    {
        auto bbf = time2bbf(time);
        char buffer[256];
        const int offset = addOneForHumanReadability ? 1 : 0;
        switch (digits) {
            case 3:
                snprintf(buffer, 256, "%d.%d.%d", offset + std::get<0>(bbf), offset + std::get<1>(bbf), std::get<2>(bbf));
                break;
            case 2:
                snprintf(buffer, 256, "%d.%d", offset + std::get<0>(bbf), offset + std::get<1>(bbf));
                break;
            case 1:
                snprintf(buffer, 256, "%d", offset + std::get<0>(bbf));
                break;
            default:
                assert(false);
        }
        return buffer;
    }

    /**
     * Quantizes an amount of time to be an even power of two of a sixteenth note.
     * Always rounds up, although returns zero if time is less than 16th note.
     */
    static float getTimeAsPowerOfTwo16th(float time)
    {
        float duration16th = 4 * time;
        if (duration16th < 1) {
            return 0;
        }

        int logTwoInt = int(std::ceil(std::log2(duration16th)));
        int num16 = int(std::round(std::exp2(logTwoInt)));
        //printf("input dur was %.2f, num 16 = %d\n", duration, num16);
        return float(num16) / 4.f;
    }
};

