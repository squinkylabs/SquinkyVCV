#pragma once

#include "SchmidtTrigger.h"

class ClockRecovery
{
public:
    ClockRecovery();
    /**
     * process one input sample.
     * return true if new period/
     */
    bool step(float);

    /**
     * Just for debugging
     */
    int _getResetCount() const;  
    float _getFrequency()const; 
    float _getEstimatedFrequency() const;    
private:

    SchmidtTrigger trigger;
};

inline ClockRecovery::ClockRecovery() : trigger(-1, 1)
{
    
}

inline bool ClockRecovery::step(float)
{
    return false;
}

inline int ClockRecovery::_getResetCount() const 
{
    return 0;
}

inline float ClockRecovery::_getFrequency() const
{
    return 0;
}

inline float ClockRecovery::_getEstimatedFrequency() const
{
    return 0;
} 
