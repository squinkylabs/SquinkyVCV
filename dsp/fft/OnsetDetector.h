#pragma once

#include "FFTData.h"

#include <memory>



class OnsetDetector
{
public:
    friend class TestOnsetDetector;
    OnsetDetector();
    bool step(float);

    static const int frameSize = 512;
    static const int preroll = 2 * frameSize;
private:
    static const int numFrames = 3;
   
    std::shared_ptr<FFTDataReal> fftFrames[numFrames];
    std::shared_ptr<FFTDataCpx> fftFramesAnalyzed[numFrames];
    int curFrame = 0;
    int indexInFrame = 0;
    int numFullFrames = 0;

    bool triggered = false;
    void analyze();

    int nextFrame();
    int prevFrame();
    int prevPrevFrame();
};
