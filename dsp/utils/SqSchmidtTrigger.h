
#pragma once

#include "SimdBlocks.h"
/**
 * This is taken from VCV rack sdk. 
 * I like their Schmidt, but I think that 0v is a uniquely bad choice for the min 
 * thershold.
 */

/*
   if (_lastOut)		// if we were true last time
        {
            if (input < _thLo) {
                _lastOut = false;
            }
        } else {
            if (input > _thHi) {
                _lastOut = true;
            }
        }
        return _lastOut;
        */
class SqSchmittTrigger {
public:
    SqSchmittTrigger() {
        reset();
    }

    void reset() {
        lastOut = float_4::mask();
    }

    float_4 process(float_4 in) {
        float_4 valIfHigh = SimdBlocks::ifelse((in < thLo), 0, lastOut);
        float_4 valIfLo = SimdBlocks::ifelse((in > thHi), SimdBlocks::maskTrue(), lastOut);
        lastOut = SimdBlocks::ifelse(lastOut, valIfHigh, valIfLo);
        return lastOut;

#if 0
        float_4 on = (in >= 2.f);
        float_4 off = (in <= 1.f);
        float_4 triggered = ~state & on;
        state = on | (state & ~off);
        return triggered;
#endif
    }

private:
    float_4 lastOut;
    const float_4 thLo = { float_4(1) };
    const float_4 thHi = { float_4(2) };
};