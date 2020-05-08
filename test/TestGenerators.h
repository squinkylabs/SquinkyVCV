#pragma once

#include <functional>

class TestGenerators
{
public: 

    using Generator = std::function<double()>;

    // This is first version. I think period is  samples, but phase is radians?
    static Generator makeSinGenerator(double periodInSamples, double initialPhase);

    /**
     * xxx Discontinuity 0..1, where 1 is two pi
     * NO - discontinuity is in radians
     */
    static Generator makeSinGeneratorPhaseJump(double periodInSamples, double initialPhase, int delay, double discontinuityRadians);
};