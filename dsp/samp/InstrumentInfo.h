#pragma once

#include <utility>

class InstrumentInfo {
public:
    using PitchRange = std::pair<int, int>;
    using KeyswitchData = std::map<std::string, PitchRange>;

    int minPitch = -1;
    int maxPitch = -1;
    KeyswitchData keyswitchData;


};
