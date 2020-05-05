
#include "OnsetDetector.h"

#include "FFTData.h"

OnsetDetector::OnsetDetector()
{
    for (int i=0; i<numFrames; ++i) {
        fftFrames[i] = std::make_shared<FFTDataReal>(frameSize);
    }
}

int OnsetDetector::nextFrame()
{
    return curFrame >= (numFrames - 1) ? 0 : curFrame + 1;
}

int OnsetDetector::prevFrame()
{
    return curFrame == 0 ? 2 : curFrame - 1;
}

int OnsetDetector::prevPrevFrame()
{
    int ret = curFrame - 2;
    if (ret < 0) {
        ret += 3;
    }
    return ret;
}

bool OnsetDetector::step(float inputData)
{
    //int curFrame = 0;
    //int indexInFrame = 0;
    fftFrames[curFrame]->set(indexInFrame, inputData);
    if (+indexInFrame >= frameSize) {
        curFrame = nextFrame();
        indexInFrame = 0;
    }
    return false;
}