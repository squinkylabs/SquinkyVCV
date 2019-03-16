#pragma once

#include "GateTrigger.h"

#include <string>
#include <vector>

class SeqClock
{
public:
    SeqClock();
    /**
     * param samplesElapsed is how many sample clocks have elapsed since last call.
     * param externalClock - is the clock CV, 0..10. will look for rising edges
     * param runStop is the run/stop CV, 0..10. will look for rising edges and toggle run state from that
     * param reset
     *
     * return - total elapsed metric time
     *
     * note that only one of the two passed params will be used, depending 
     * on internal/external model.
     */
    double update(int samplesElapsed, float externalClock, float runStop, float reset);
    void setup(int inputSetting, float tempoSetting, float sampleTime);
    void reset();
    static std::vector<std::string> getClockRates();

    double getCurMetricTime() const {
        return curMetricTime; 
    }
private:
    int clockSetting = 0;       // default to internal
    float internalTempo = 120.f;
    double curMetricTime = 0;
    float sampleTime = 1.f / 44100.f;
    double metricTimePerClock = 1;

    GateTrigger clockProcessor;
    GateTrigger runProcessor;
    GateTrigger resetProcessor;
};

inline SeqClock::SeqClock() :
    clockProcessor(true),
    runProcessor(true),
    resetProcessor(true)
{

}

inline double SeqClock::update(int samplesElapsed, float externalClock, float runStop, float reset)
{
    // Internal clock
    if (clockSetting == 0) {
        double deltaSeconds = samplesElapsed * sampleTime;
        double deltaMetric = deltaSeconds * internalTempo / 60.0;
        curMetricTime += deltaMetric;
    } else {
        // external clock
        clockProcessor.go(externalClock);
        if (clockProcessor.trigger()) {
            curMetricTime += metricTimePerClock;
        }
    }
    return curMetricTime;
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
    switch (clockSetting) {
        case 0:
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
