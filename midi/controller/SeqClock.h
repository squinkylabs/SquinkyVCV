#pragma once

#include "GateTrigger.h"
#include "OneShot.h"

#include <string>
#include <vector>

class SeqClock
{
public:
    SeqClock();

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
    void setup(int inputSetting, float tempoSetting, float sampleTime);
    void reset();
    static std::vector<std::string> getClockRates();

    double getCurMetricTime() const {
        return curMetricTime; 
    }

    double getMetricTimePerClock() {
        return metricTimePerClock;
    }

   // void setSampleTime(float);
private:
    int clockSetting = 0;       // default to internal
    float internalTempo = 120.f;
    double curMetricTime = 0;
    float sampleTime = 1.f / 44100.f;
    double metricTimePerClock = 1;

    GateTrigger clockProcessor;
    GateTrigger resetProcessor;
    OneShot resetLockout;
};

inline SeqClock::SeqClock() :
    clockProcessor(true),
    resetProcessor(true)
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
        curMetricTime = 0;          // go back to start
    }
    for (int i = 0; i < samplesElapsed; ++i) {
        resetLockout.step();
    }

    if (!runStop) {
        results.totalElapsedTime = curMetricTime;
        return results;
    }
    // Internal clock
    if (clockSetting == 0) {
        double deltaSeconds = samplesElapsed * sampleTime;
        double deltaMetric = deltaSeconds * internalTempo / 60.0;
        curMetricTime += deltaMetric;
    } else {
        // ignore external clock during lockout.
        if (resetLockout.hasFired()) {
            // external clock
            clockProcessor.go(externalClock);
            if (clockProcessor.trigger()) {
                curMetricTime += metricTimePerClock;
            }
        }
    }
    results.totalElapsedTime = curMetricTime;
    return results;
}

inline void SeqClock::reset()
{
    curMetricTime = 0;
}

inline void SeqClock::setup(int inputSetting, float tempoSetting, float sampleT)
{
    internalTempo = tempoSetting;
    sampleTime = sampleT;
    clockSetting = inputSetting;
    resetLockout.setSampleTime(sampleT);
    switch (clockSetting) {
        case 0:
            // just a hack, there really isn't a known value
            // for internal clock, but this may make external clients
            //happy
            metricTimePerClock = .005;     
            break;
        case 1:
            metricTimePerClock = .0625;
            break;
        case 2:
            metricTimePerClock = .125;
            break;
        case 3:
            metricTimePerClock = .25;
            break;
        case 4: 
            metricTimePerClock = .5;
            break;
        case 5:
            metricTimePerClock = 1;
            break;
        default:
            assert(false);
    }
}

inline std::vector<std::string> SeqClock::getClockRates()
{
    return {
        "Internal",
        "64th note",
        "32nd note",
        "16th note",
        "8th note",
        "Quarter"
    };
}
