
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
        printf("will do to polar on %d\n", curFrame);
        fftFramesAnalyzed[curFrame]->toPolar();
        analyze();
        curFrame = nextFrame();
        indexInFrame = 0;
    }
    return false;
}

void OnsetDetector::analyze()
{
    printf("enter analyze, ff=%d\n", numFullFrames);
    if (numFullFrames < 3) {
        return;
    }
    FFTUtils::Stats stats;
    FFTUtils::getStats(stats, *fftFramesAnalyzed[prevPrevFrame()], *fftFramesAnalyzed[prevFrame()], *fftFramesAnalyzed[curFrame]);
    printf("analyze frame, jump = %f\n", stats.averagePhaseJump);
    if (stats.averagePhaseJump > .1) {
        triggered = true;
    }
    printf("will clear polary on %d\n", prevPrevFrame());
    fftFramesAnalyzed[prevPrevFrame()]->reset();
 
}