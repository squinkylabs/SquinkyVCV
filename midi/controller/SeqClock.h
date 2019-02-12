#pragma once

#include <vector>

class SeqClock
{
public:
    /**
     * return - total elapsed metric time
     */
    double update(int samplesElapsed, bool externalClock);
    void setup(int inputSetting, float tempoSetting, float sampleTime);
    void reset();
    static std::vector<std::string> getClockRates();
private:
    int clockSetting = 0;       // default to internal
    float internalTempo = 120.f;
    double curMetricTime = 0;
    float sampleTime = 1.f / 44100.f;

    
};

inline double SeqClock::update(int samplesElapsed, bool externalClock)
{
    if (clockSetting == 0) {
        double deltaSeconds = samplesElapsed * sampleTime;
        double deltaMetric = deltaSeconds * internalTempo / 60.0;
        curMetricTime += deltaMetric;
    } else {
        assert(false);
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
