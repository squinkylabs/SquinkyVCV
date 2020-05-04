
#include "FFTUtils.h"
#include "AudioMath.h"

class GeneratorImp
{
public:
    GeneratorImp(double periodInSamples, double initialPhase) : 
        phaseInc(AudioMath::_2Pi / periodInSamples), 
        phaseRadians(initialPhase)
    {
    }
    double phaseRadians = 0;
    const double phaseInc;
};

FFTUtils::Generator FFTUtils::makeSinGenerator(double periodInSamples, double initialPhase)
{
    std::shared_ptr<GeneratorImp> impl = std::make_shared<GeneratorImp>(periodInSamples, initialPhase);
    double phaseInc = 1.f / periodInSamples;
    Generator g =  [impl]() {
        // TODO: get rid of static
        // static double phase = 0;
        double ret =  std::sin(impl->phaseRadians);
        impl->phaseRadians += impl->phaseInc;
        if (impl->phaseRadians >= AudioMath::_2Pi) {
            impl->phaseRadians -= AudioMath::_2Pi;
        }
        return ret;
    };
    return g;
}

class GeneratorJumpImp
{
public:
    GeneratorJumpImp(double periodInSamples, double initialPhase, int delay, double _discontinuity) : 
        phase(initialPhase),
        delayCounter(delay), 
        phaseInc(AudioMath::_2Pi / periodInSamples),
        discontinuity(_discontinuity)
    {
    }

    double phase = 0;
    const double phaseInc;
    int delayCounter;
    const double discontinuity;
};

FFTUtils::Generator FFTUtils::makeSinGeneratorPhaseJump(double periodInSamples, double initialPhase, int delay, double discontinuity)
{
    std::shared_ptr<GeneratorJumpImp> impl = std::make_shared<GeneratorJumpImp>(periodInSamples, initialPhase, delay, discontinuity);
    Generator g = [impl]() {
        double ret = std::sin(impl->phase);
        impl->phase += impl->phaseInc;
        if (--impl->delayCounter == 0) {
            impl->phase += impl->discontinuity;
        }
        if (impl->phase >= AudioMath::_2Pi ) {
            impl->phase -= AudioMath::_2Pi ;
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
    double magSum = 0;
    double weightedJumpSum = 0;
    
    for (int bin = 0; bin < a.size(); ++bin) {

        auto mpa = a.getMagAndPhase(bin);
        auto mpb = b.getMagAndPhase(bin);
        auto mpc = c.getMagAndPhase(bin);

        const double phaseDiff0 = PhaseAngleUtil::distance(mpb.second, mpa.second);
        const double phaseDiff1 = PhaseAngleUtil::distance(mpc.second, mpb.second);

        const double mag = mpa.first;
        const double jump = std::abs(PhaseAngleUtil::distance(phaseDiff1,  phaseDiff0));

        if (mag > .005) {
         //   printf("bin %d mag %f jump=%.2f, ph = %.2f, %.2f, %.2f\n", bin, mag, jump, mpa.second, mpb.second, mpc.second);
        }
        assert(mag >= 0);
        magSum += mag;
        weightedJumpSum += (jump * mag);

    }
   // printf("total shift %f mag %f\n", weightedJumpSum, magSum);
    double totalJump = (magSum > 0) ? weightedJumpSum / magSum : 0;
    stats.largestPhaseJump = totalJump;
    
}
