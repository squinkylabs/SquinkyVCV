#pragma once

#include "SqPort.h"
class ACDetector {
public:
    bool step(SqInput& input, int baseChannel);
private:
};

inline bool ACDetector::step(SqInput& input, int baseChannel)
{
    return false;
}