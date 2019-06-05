#include "Analyzer.h"
#include "LadderFilter.h"
#include <stdio.h>
#include <memory>

using T = float;
using Filt = LadderFilter<T>;
using FiltPtr = std::shared_ptr<Filt>;

const int numBins = 4096;

FiltPtr getFilter(float normFc, float feedback)
{
    std::shared_ptr < LadderFilter<float>> filter = std::make_shared<LadderFilter<float>>();
    filter->setFeedback(feedback);
    filter->setNormalizedFc(normFc);
    filter->setVoicing(Filt::Voicing::Clean);
    return filter;
}

double getPeakAmp(FiltPtr filter)
{
    std::function<float(float)> doFilter = [filter](float x) {
        filter->run(x);
        return filter->getOutput();
    };
    FFTDataCpx fftData(numBins);
    Analyzer::getFreqResponse(fftData, doFilter);
    int maxBin = Analyzer::getMax(fftData);
//    static double bin2Freq(int bin, double sampleRate, int numBins);
    double freq = FFT::bin2Freq(maxBin, 44100, numBins);
    double amp = fftData.getAbs(maxBin);
    printf("bin = %d, freq = %f amp = %f\n", maxBin, freq, amp);
    return freq;
}

double getPeakAmp(double normFc, double feedback)
{
    printf("\n");
    double ret = 0;

    
    auto filter = getFilter(T(normFc), T(feedback));
    ret = getPeakAmp(filter);

    printf("getPeakAmp(%f, %f) = %f\n", normFc, feedback, ret);
    return ret;
}


void calQ()
{
    getPeakAmp(.01, 3.0);
    getPeakAmp(.1, 3.0);
    getPeakAmp(.3, 3.0);
}