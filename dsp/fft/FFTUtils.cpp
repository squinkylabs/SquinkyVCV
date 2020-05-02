
#include "FFTUtils.h"
#include "AudioMath.h"

FFTUtils::Generator FFTUtils::makeSinGenerator(double periodInSamples)
{
    printf("making regular generator\n");
    double phaseInc = 1.f / periodInSamples;
    Generator g =  [phaseInc]() {
        // TODO: get rid of static
        static double phase = 0;
        double ret =  std::sin(phase * 2.f * float(AudioMath::Pi));
        phase += phaseInc;
        if (phase >= 1) {
            phase -= 1;
        }
        return ret;
    };
    return g;
}


class GeneratorJumpImp
{
public:
    GeneratorJumpImp(double periodInSamples, int delay, double _discontinuity) : 
        delayCounter(delay), 
        phaseInc(1.0 / periodInSamples),
        discontinuity(_discontinuity)
    {

    }

    double phase = 0;
    const double phaseInc;
    int delayCounter;
    const double discontinuity;

};
FFTUtils::Generator FFTUtils::makeSinGeneratorPhaseJump(double periodInSamples, int delay, double discontinuity)
{
    printf("making generator with delay = %d, disc = %f\n", delay, discontinuity);
   // float phaseInc = 1.f / periodInSamples;

    std::shared_ptr<GeneratorJumpImp> impl = std::make_shared<GeneratorJumpImp>(periodInSamples, delay, discontinuity);
    Generator g = [impl]() {
    //    static int delayCounter = delay;
      //  printf("in the lambda, delayctr = %d\n", delayCounter);
    //    static float phase = 0;
    //    const static float phaseInc = 1.f / periodInSamples;
        double ret = std::sin(impl->phase * 2.f * float(AudioMath::Pi));
        impl->phase += impl->phaseInc;
       // printf("gen loop, delay ctr = %d\n", delayCounter);
        if (--impl->delayCounter == 0) {
            printf("jumping phase\n");
            impl->phase += impl->discontinuity;
        }
        if (impl->phase >= 1) {
            impl->phase -= 1;
        }
        return float(ret);
    };
    return g;
}

std::vector< FFTDataCpxPtr> FFTUtils::generateFFTs(int numSamples, int frameSize, std::function<double()> generator)
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

std::vector< FFTDataRealPtr> FFTUtils::generateData(int numSamples, int frameSize, std::function<double()> generator)
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
        float x = (float) generator();
        buffer->set(index, x);
        ++index;
        if (index >= frameSize) {
            buffer.reset();
        }
    }
    return ret;
}

void FFTUtils::getStats(Stats& stats, const FFTDataCpx& a, const FFTDataCpx& b, const FFTDataCpx& c)
{
    assert(a.size() == b.size());
    assert(a.size() == c.size());
    assert(a.isPolar() && b.isPolar() && c.isPolar());

    double biggestJump = 0;
    for (int bin = 0; bin < a.size(); ++bin) {
        double phaseDiff0 = b.getMagAndPhase(bin).second - a.getMagAndPhase(bin).second;
        double phaseDiff1 = c.getMagAndPhase(bin).second - b.getMagAndPhase(bin).second;

        const double mag = a.getMagAndPhase(bin).first;
        if (mag > .01) {
            printf("bin %d mag %f ph = %f, %f, %f\n", bin, mag, a.getMagAndPhase(bin).second, b.getMagAndPhase(bin).second, c.getMagAndPhase(bin).second);
        }
        double jump = std::abs(phaseDiff1 - phaseDiff0);
        biggestJump = std::max(jump, biggestJump);
    }
    stats.largestPhaseJump = biggestJump;
    
}