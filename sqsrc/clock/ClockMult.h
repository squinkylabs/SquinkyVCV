#pragma once


class ClockMult
{
public:
    /**
     * Clocks the multiplier by one sample.
     * Returns 0 or more high speed clocks.
     */
    int sampleClock();

    /**
     * Sends one reference tick to the multiplier
     */
    void refClock();

    /** 
     * When a ref count comes in early, instead of puking out a ton of
     * sample clocks to keep up, we instead reset. Will immediately clear after call.	
     */
    bool getReset();

    bool getMultipliedClock();

    void setDivisor(int);

};