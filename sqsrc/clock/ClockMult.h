#pragma once


class ClockMult
{
public:
    /**
     * Clocks the multiplier by one sample.
     * Returns 0 or more high speed clocks.
     */
    void sampleClock();

    /**
     * Sends one reference tick to the multiplier. This is
     * the "input" to the multiplier.
     */
    void refClock();

    /** 
     * 0..1 saw at the same rate as multiplier output.	
     */
    float getSaw() const
    {
        return sawPhase;
    }

    bool getMultipliedClock() const
    {
        return clockOutValue;
    }

    void setDivisor(int);

    float _getFreq() const
    {
        return learnedFrequency;
    }
private:
    enum class State
    {
        INIT,
        TRAINING,
        RUNNING
    };
    int trainingCounter = 12345;
    int learnedPeriod = 999;
    float learnedFrequency = 0;
    State state = State::INIT;

    bool clockOutValue = 0;
    int clockOutTimer = 0;

    float sawPhase = 0;

    void startNewClock();

};