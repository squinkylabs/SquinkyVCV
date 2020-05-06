
#include "OnsetDetector.h"

#include "FFTData.h"
#include "FFTUtils.h"

OnsetDetector::OnsetDetector()
{
    for (int i=0; i<numFrames; ++i) {
        fftFrames[i] = std::make_shared<FFTDataReal>(frameSize);
        fftFramesAnalyzed[i] = std::make_shared<FFTDataCpx>(frameSize);
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
    if (++indexInFrame >= frameSize) {
        numFullFrames++;
        // now take fft into cpx
        FFT::forward(fftFramesAnalyzed[curFrame].get(), *fftFrames[curFrame]);
        fftFramesAnalyzed[curFrame]->toPolar();
        analyze();
        curFrame = nextFrame();
        indexInFrame = 0;
    }
    return false;
}

void OnsetDetector::analyze()
{
    if (numFullFrames < 3) {
        return;
    }
    FFTUtils::Stats stats;
    FFTUtils::getStats(stats, *fftFramesAnalyzed[prevPrevFrame()], *fftFramesAnalyzed[prevFrame()], *fftFramesAnalyzed[curFrame]);
    assert(false);
}