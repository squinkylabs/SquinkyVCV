
#include "TestGenerators.h"

#include "AudioMath.h"

#include "asserts.h"
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


TestGenerators::Generator TestGenerators::makeStepGenerator(int stepPos)
{
    std::shared_ptr<int> counter = std::make_shared<int>(0);
    return [counter, stepPos]() {
        float ret = -1;
       
        if (*counter < stepPos) {
            ret = 0;
        } else if (*counter < stepPos + TestGenerators::stepDur) {
            ret = 1;
        } else {
            ret = 0;
        }
        ++* counter;
       
        return ret;
    };
}

TestGenerators::Generator TestGenerators::makeSteppedSinGenerator(int stepPos, double normalizedFreq, double stepGain)
{
    assert(stepGain > 1);
    static float lastOut = 0;

    std::shared_ptr<int> counter = std::make_shared<int>(0);
    std::shared_ptr<double> phase = std::make_shared<double>(0);

    return [counter, stepPos, phase, normalizedFreq, stepGain]() {

        *phase += normalizedFreq;
        if (*phase >= AudioMath::_2Pi) {
            *phase -= AudioMath::_2Pi;
        }

        double gain = 1;

        if (*counter < stepPos) {
            gain = 1 / stepGain;
        }
        else if (*counter < stepPos + stepDur) {
            gain = 1;
        }
        else {
            gain = 1 / stepGain;
        }
        ++* counter;

#if 0
        if (*counter < 100) {
            printf("ph = %.2f sin= %.2f will ret %.2f\n",
                *phase,
                std::sin(*phase),
                (gain * std::sin(*phase)));
        }
#endif
        float ret = float(gain * std::sin(*phase));
        assert(ret < 1);
        assert(ret > -1);

        assertLT(fabs(ret - lastOut) , .04);
        lastOut = ret;

        return ret;
    };
}

TestGenerators::Generator TestGenerators::makeSinGenerator(double normalizedFreq)
{
    static float lastOut = 0;
    assert(normalizedFreq < AudioMath::Pi);
    assert(normalizedFreq > 0);

    std::shared_ptr<double> phase = std::make_shared<double>(0);

    return [phase, normalizedFreq]() {

        *phase += normalizedFreq;
        if (*phase >= AudioMath::_2Pi) {
            *phase -= AudioMath::_2Pi;
        }

        double ret = std::sin(*phase);
        assert(ret <= 1);
        assert(ret >= -1);

        return float(ret);
    };
}