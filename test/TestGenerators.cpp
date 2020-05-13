
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
    ~GeneratorImp() {
        printf("biggest sin jump = %f\n", biggestJump);
    }
    double phaseRadians = 0;
    const double phaseInc;

    // for debugging
    double biggestJump = 0;
    double lastOutput = 0;
};

TestGenerators::Generator TestGenerators::makeSinGenerator(double periodInSamples, double initialPhase)
{
    std::shared_ptr<GeneratorImp> impl = std::make_shared<GeneratorImp>(periodInSamples, initialPhase);
    printf("in  TestGenerators::makeSinGenerator periodin samp = %f phaseInc = %f\n", periodInSamples, impl->phaseInc);
    Generator g =  [impl]() {
        // TODO: get rid of static
        // static double phase = 0;
        double ret =  std::sin(impl->phaseRadians);
        impl->phaseRadians += impl->phaseInc;
        if (impl->phaseRadians >= AudioMath::_2Pi) {
            impl->phaseRadians -= AudioMath::_2Pi;
        }
        double jump = std::abs(ret - impl->lastOutput);
        impl->biggestJump = std::max(jump, impl->biggestJump);
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

// TODO: this is a saw, not a sin
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

// TODO: this is a saw generator
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

#if 0


class Osc3
{
public:
    Osc3(double freq) {
        setFreq(freq);
    }
    void setFreq(double t)
    {
        assert(t > 0 && t < .51);
        //   tapWeight = t * (1.0 / AudioMath::Pi);
        tapWeight = 2 * sin(t);
    }
    double gen()
    {
        const double x = zX + tapWeight * zY;
        const double y = -(tapWeight * x) + zY;

        zX = x;
        zY = y;
        return zX;
    }

 //   const double k = sqrt(2);

    double zX = 1;
    double zY = 1;
    double tapWeight = .1;
};

class Old
{
public:
    Old(double freq) 
    {
        A = 2 * std::cos(freq);
        yn_minus_one = -std::sin(freq);
        yn_minus_two = -std::sin(2 * freq);
    }
    double gen()
    {
        double y = (A * yn_minus_one) - yn_minus_two;
        yn_minus_two = yn_minus_one;
        yn_minus_one = y;
        return y;
    }
private:
    double yn_minus_one = 0;
    double yn_minus_two = 0;
    double A = 0;
};

TestGenerators::Generator TestGenerators::makeSinGenerator(double freqRadians)
{
    static float lastOut = 0;
    assert(freqRadians < AudioMath::Pi);
    assert(freqRadians > 0);

    std::shared_ptr<Old> imp = std::make_shared<Old>(freqRadians);
    return [imp]() {
        return imp->gen();
    };
}

#else
// first version - standard phase accumulator
TestGenerators::Generator TestGenerators::makeSinGenerator(double normalizedFreq)
{
    static float lastOut = 0;
    assert(normalizedFreq < AudioMath::Pi);
    assert(normalizedFreq > 0);

    printf("in makeSinGenerator phase inc IS normFreq = %f\n", normalizedFreq);
    std::shared_ptr<double> phase = std::make_shared<double>(0);

    return [phase, normalizedFreq]() {

        *phase += normalizedFreq;
        if (*phase > AudioMath::_2Pi) {
            *phase -= AudioMath::_2Pi;
        }

        double ret = std::cos(*phase);
        assert(ret <= 1);
        assert(ret >= -1);

        return float(ret);
    };
}
#endif