
#include "FFTUtils.h"
#include "AudioMath.h"

FFTUtils::Generator FFTUtils::makeSineGenerator(float periodInSamples)
{
    float phaseInc = 1.f / periodInSamples;
    Generator g =  [phaseInc]() {
        static float phase = 0;
        float ret =  std::sin(phase * 2.f * float(AudioMath::Pi));
        phase += phaseInc;
        if (phase >= 1) {
            phase -= 1;
        }
        return ret;
    };
    return g;
}

FFTUtils::Generator FFTUtils::makeSineGeneratorPhaseJump(float periodInSamples, int delay, float discontinuity)
{
   // float phaseInc = 1.f / periodInSamples;
    Generator g = [periodInSamples, delay, discontinuity]() {
        static int delayCounter = delay;
        static float phase = 0;
        const static float phaseInc = 1.f / periodInSamples;
        float ret = std::sin(phase * 2.f * float(AudioMath::Pi));
        phase += phaseInc;
        if (--delayCounter == 0) {
            phase += discontinuity;
        }
        if (phase >= 1) {
            phase -= 1;
        }
        return ret;
    };
    return g;
}

std::vector< FFTDataCpxPtr> FFTUtils::generateFFTs(int numSamples, int frameSize, std::function<float()> generator)
{
    auto data = generateData(numSamples, frameSize, generator);
    std::vector<FFTDataCpxPtr> ret;
    for (auto buffer : data) {
        FFTDataCpxPtr  fft = std::make_shared<FFTDataCpx>(frameSize);
        FFT::forward(fft.get(), *buffer);
        ret.push_back(fft);
    }
    return ret;
}

std::vector< FFTDataRealPtr> FFTUtils::generateData(int numSamples, int frameSize, std::function<float()> generator)
{
    std::vector< FFTDataRealPtr> ret;
    FFTDataRealPtr buffer;
    int index = 0;
    while (numSamples--) {
        if (!buffer) {
            buffer = std::make_shared<FFTDataReal>(frameSize);
            ret.push_back(buffer);
            index = 0;
        }
        float x = generator();
        buffer->set(index, x);
        ++index;
        if (index >= frameSize) {
            buffer.reset();
        }
    }
    return ret;
}

void FFTUtils::getStats(Stats&, const FFTDataCpx& a, const FFTDataCpx& b, const FFTDataCpx& c)
{

}