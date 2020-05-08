
#include "TestGenerators.h"

#include "AudioMath.h"
#include <memory>

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

TestGenerators::Generator TestGenerators::makeSinGenerator(double periodInSamples, double initialPhase)
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

TestGenerators::Generator TestGenerators::makeSinGeneratorPhaseJump(double periodInSamples, double initialPhase, int delay, double discontinuity)
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
