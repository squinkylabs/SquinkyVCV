
#pragma once

#include <vector>

class SimpleQuantizer
{
public:
    enum Scales {
        _12Even,
        _8Diatonic,
        _12Just,
        _8Just
    };
    SimpleQuantizer(std::vector<Scales> scales, Scales scale) {
        
    }

    float quantize(float);
private:
};