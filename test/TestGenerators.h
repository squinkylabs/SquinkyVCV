#pragma once

#include <functional>

class TestGenerators
{
public: 

    using Generator = std::function<double()>;

    /**
     * These are the original ones, for the low level tests
     */

    // This is first version. I think period is  samples, but phase is radians?
    static Generator makeSinGenerator(double periodInSamples, double initialPhase);

    /**
     * xxx Discontinuity 0..1, where 1 is two pi
     * NO - discontinuity is in radians
     */
    static Generator makeSinGeneratorPhaseJump(double periodInSamples, double initialPhase, int delay, double discontinuityRadians);

    /**
     * 
     * These ones made for highter level tests - should combine
     */
    
    static Generator makeStepGenerator(int stepPos);

    static Generator makeSteppedSinGenerator(int stepPos, double normalizedFreq, double stepGain);

    static Generator makeSinGenerator(double normalizedFreq);

    const static int stepDur = 100;
    const static int stepTail = 1024;
};