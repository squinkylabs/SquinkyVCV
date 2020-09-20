#pragma once

class ClockRecovery
{
public:
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
};

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
