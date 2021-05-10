#pragma once

#include <string>
class Comp2TextUtil {
public:
    static std::string modeTest(int labelMode);
    static std::string channelLabel(int mode, int channel);
    static std::string channelModeMenuLabel(int mode, int stereo);
};
