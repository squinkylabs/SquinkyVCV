#pragma once

#include "FFTData.h"

#include <memory>



class OnsetDetector
{
public:
    friend class TestOnsetDetector;
    OnsetDetector();
    bool step(float);
private:
    static const int numFrames = 3;
    static const int frameSize = 512;
    std::shared_ptr<FFTDataReal> fftFrames[3];
    int curFrame = 0;
    int indexInFrame = 0;

    int nextFrame();
    int prevFrame();
    int prevPrevFrame();
};
