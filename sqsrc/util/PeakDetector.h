#pragma once

#include "SimdBlocks.h"
#include "SqLog.h"

class PeakDetector
{
public:
    void step(float x)
    {
        value = std::max(value, x);
    }
    float get() const
    {
        return value;
    }
    void decay(float sec)
    {
        value -= 4 * sec;
        value = std::max(value, 0.f);
    }
private:
    float value = 0;
};

class PeakDetector4
{
public:
    void step(float_4 x)
    {
        buffer = SimdBlocks::max(buffer, x);
    }

    float get() const
    {
        return value;
    }
    /**
     * transfers buffer into value
     * then decays value
     */
    void decay(float sec)
    {
        value = std::max(value, buffer[0]);
        value = std::max(value, buffer[1]);
        value = std::max(value, buffer[2]);
        value = std::max(value, buffer[3]);
        buffer = float_4(0);
     //   SQINFO("rounded up another value %.2f", value);
        
        value -= decayRate * sec;
        value = std::max(value, 0.f);
      //  SQINFO("decayed = %.2f", value);
    }

    void setDecay(float d) {
        decayRate = d;
    }
private:
    float decayRate = 0;
    float value = 0;
    float_4 buffer = {0};
};
