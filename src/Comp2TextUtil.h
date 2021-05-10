#pragma once

#include <string>

#include "SqStream.h"

class Comp2TextUtil {
public:
    static std::string stereoModeText(int stereo);
    static std::string channelLabel(int mode, int channel);
    static std::string channelModeMenuLabel(int mode, bool isStereo);
};

std::string Comp2TextUtil::channelModeMenuLabel(int mode, bool isStereo) {
    switch (mode) {
        case 0:
            return isStereo ? "Channels: 1-8" : "Channels: 1-16";
            break;
        case 1:
            return isStereo ? "Channels: 9-16" : "Channels: 1-16";
            break;
        case 2:
            return "Channels: Group/Aux";
            break;
    }
}

std::string Comp2TextUtil::stereoModeText(int stereo) {
    switch (stereo) {
        case 0:
            return "Mode: multi-mono";
            break;
        case 1:
            return "Mode: stereo";
            break;
        case 2:
            return "Mode: linked-stereo";
            break;
        default:
            assert(false);
            return "";
    }
}

std::string Comp2TextUtil::channelLabel(int mode, int channel) {
    SqStream sq;
    switch (mode) {
        case 0:
            sq.add(channel);
            break;
        case 1:
            sq.add(channel + 8);
            break;
        case 2: {
            switch (channel) {
                case 1:
                    sq.add("G1");
                    break;
                case 2:
                    sq.add("G2");
                    break;
                case 3:
                    sq.add("G3");
                    break;
                case 4:
                    sq.add("G4");
                    break;
                case 5:
                    sq.add("A1");
                    break;
                case 6:
                    sq.add("A2");
                    break;
                case 7:
                    sq.add("A3");
                    break;
                case 8:
                    sq.add("A4");
                    break;
                default:
                    FATAL("channel out of range %d", channel);
                    assert(false);
            }
        } break;
    }
    return sq.str();
}