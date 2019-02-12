#pragma once

#include <vector>

class SeqClock
{
public:
    void update(int samplesElapsed, bool externalClock);
    void setup(int inputSetting, float tempoSetting);
    static std::vector<std::string> getClockRates();
};

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
