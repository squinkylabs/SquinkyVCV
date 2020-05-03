
#include "FFTUtils.h"
#include "AudioMath.h"

class GeneratorImp
{
public:
    GeneratorImp(double periodInSamples) : phaseInc(AudioMath::_2Pi / periodInSamples)
    {
    }
    double phaseRadians = 0;
    const double phaseInc;
};

FFTUtils::Generator FFTUtils::makeSinGenerator(double periodInSamples)
{
    std::shared_ptr<GeneratorImp> impl = std::make_shared<GeneratorImp>(periodInSamples);
    printf("making regular generator\n");
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
    GeneratorJumpImp(double periodInSamples, int delay, double _discontinuity) : 
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

FFTUtils::Generator FFTUtils::makeSinGeneratorPhaseJump(double periodInSamples, int delay, double discontinuity)
{
    printf("making generator with delay = %d, disc = %f\n", delay, discontinuity);

    std::shared_ptr<GeneratorJumpImp> impl = std::make_shared<GeneratorJumpImp>(periodInSamples, delay, discontinuity);
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
    printf("** getStats\n");
    assert(a.size() == b.size());
    assert(a.size() == c.size());
    assert(a.isPolar() && b.isPolar() && c.isPolar());

    double biggestJump = 0;
    for (int bin = 0; bin < a.size(); ++bin) {

        auto mpa = a.getMagAndPhase(bin);
        auto mpb = b.getMagAndPhase(bin);
        auto mpc = c.getMagAndPhase(bin);

        //assert(PhaseAngleUtil::isNormalized(mpa.second));
      //  assert(PhaseAngleUtil::isNormalized(mpb.second));
       // assert(PhaseAngleUtil::isNormalized(mpc.second));

        const double phaseDiff0 = PhaseAngleUtil::distance(mpb.second, mpa.second);
        const double phaseDiff1 = PhaseAngleUtil::distance(mpc.second, mpb.second);
     //   double phaseDiff0 = b.getMagAndPhase(bin).second - a.getMagAndPhase(bin).second;
    //    double phaseDiff1 = c.getMagAndPhase(bin).second - b.getMagAndPhase(bin).second;

        const double mag = mpa.first;
        if (mag > .01) {
            printf("bin %d mag %f ph = %f, %f, %f\n", bin, mag,mpa.second, mpb.second, mpc.second);
        }
      //  double jump = std::abs(phaseDiff1 - phaseDiff0);
        const double jump = PhaseAngleUtil::distance(phaseDiff1,  phaseDiff0);
        biggestJump = std::max(jump, biggestJump);
    }
    stats.largestPhaseJump = biggestJump;
    
}