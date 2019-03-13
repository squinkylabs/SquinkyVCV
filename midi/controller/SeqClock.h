#pragma once

#include <vector>

class SeqClock
{
public:
    /**
     * param samplesElapsed is how many sample clocks have elapsed since last call.
     * param externalClock - if true indicates a rising edge of clock
     * return - total elapsed metric time
     *
     * note that only one of the two passed params will be used, depending 
     * on internal/external model.
     */
    double update(int samplesElapsed, bool externalClock);
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
};

inline double SeqClock::update(int samplesElapsed, bool externalClock)
{
    if (clockSetting == 0) {
        double deltaSeconds = samplesElapsed * sampleTime;
        double deltaMetric = deltaSeconds * internalTempo / 60.0;
        curMetricTime += deltaMetric;
    } else {
        if (externalClock) {
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
