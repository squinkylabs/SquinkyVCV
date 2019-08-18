#pragma once

#include "GateTrigger.h"
#include "OneShot.h"

#include <string>
#include <vector>

class SeqClock
{
public:
    SeqClock();

    enum class ClockRate
    {
    //    Internal,
        Div64,
        Div32,
        Div16,        // sixty-fourth
        Div8,       // thirty second
        Div4,       // sixteenth
        Div2,       // eighth
        Div1,       // quarter
        NUM_CLOCKS
    };

    /**
     * Data returned when clock is advanced
     */
    class ClockResults
    {
    public:
        double totalElapsedTime = 0;
        bool didReset = false;
    };

    /**
     * param samplesElapsed is how many sample clocks have elapsed since last call.
     * param externalClock - is the clock CV, 0..10. will look for rising edges
     * param runStop is the run/stop flag. External logic must toggle it. It is level
     *      sensitive, so clock stays stopped as long as it is low
     * param reset is edge sensitive. Only false -> true transition will trigger a reset.
     *
     * return - total elapsed metric time
     *
     * note that only one of the two passed params will be used, depending 
     * on internal/external model.
     */
    ClockResults update(int samplesElapsed, float externalClock, bool runStop, float reset);

    void setup(ClockRate inputSetting, float, float sampleTime);
    void reset(bool internalClock);

    static std::vector<std::string> getClockRates();

    double getCurMetricTime() const {
        return curMetricTime; 
    }

    double getMetricTimePerClock() {
        return metricTimePerClock;
    }

   // void setSampleTime(float);
private:
    ClockRate clockSetting = ClockRate::Div4;
   // float internalTempo = 120.f;
    double curMetricTime = -1;                          // this is correct for external, who cares about internal?          
    float sampleTime = 1.f / 44100.f;
    double metricTimePerClock = 1;

    GateTrigger clockProcessor;
    GateTrigger resetProcessor;
    OneShot resetLockout;
};

// We don't want reset logic on clock, as clock high should not be ignoreed.
// Probably don't want on reset either.
inline SeqClock::SeqClock() :
    clockProcessor(false),
    resetProcessor(false)
{
    resetLockout.setDelayMs(1);
    resetLockout.setSampleTime(1.f / 44100.f);
}

inline SeqClock::ClockResults SeqClock::update(int samplesElapsed, float externalClock, bool runStop, float reset)
{
    ClockResults results;
    // if stopped, don't do anything

    resetProcessor.go(reset);
    results.didReset = resetProcessor.trigger();
    if (results.didReset) {
        resetLockout.set();
        // go back to start. For correct start, go negative, so that first clock plays first note
        curMetricTime = -1;

        // reset the clock so that high clock can gen another clock
        clockProcessor.reset();
    }
    for (int i = 0; i < samplesElapsed; ++i) {
        resetLockout.step();            // TODO: don't iterate
    }

    if (!runStop) {
        results.totalElapsedTime = curMetricTime;
        return results;
    }

    // ignore external clock during lockout
    if (resetLockout.hasFired()) {
        // external clock
        clockProcessor.go(externalClock);
        if (clockProcessor.trigger()) {
            //printf("seqClock proc new one\n"); fflush(stdout);
            // if an external clock fires, advance the time.
            // But if we are reset (negative time), then always go to zero
            if (curMetricTime >= 0) {
                curMetricTime += metricTimePerClock;
            } else {
                curMetricTime = 0;
            }
        }
    }

    results.totalElapsedTime = curMetricTime;
    return results;
}

inline void SeqClock::reset(bool internalClock)
{
    curMetricTime = internalClock? 0 : -1;
}

inline void SeqClock::setup(ClockRate inputSetting, float, float sampleT)
{
  //  internalTempo = tempoSetting;
    sampleTime = sampleT;
    clockSetting = inputSetting;
    resetLockout.setSampleTime(sampleT);
    switch (clockSetting) {
        case  ClockRate::Div64:
            metricTimePerClock = .0625 / 4.0;
            break;
        case  ClockRate::Div32:
            metricTimePerClock = .0625 / 2.0;
            break;
        case  ClockRate::Div16:
            metricTimePerClock = .0625;
            break;
        case  ClockRate::Div8:
            metricTimePerClock = .125;
            break;
        case  ClockRate::Div4:
            metricTimePerClock = .25;
            break;
        case  ClockRate::Div2:
            metricTimePerClock = .5;
            break;
        case ClockRate::Div1:
            metricTimePerClock = 1;
            break;
        default:
            assert(false);
    }
}

inline std::vector<std::string> SeqClock::getClockRates()
{
    return {
        "x64",
        "x32",
        "x16 64th",
        "x8 32nd",
        "x4 16th",
        "x2 8th",
        "x1 Quarter"
    };
}
