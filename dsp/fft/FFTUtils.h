
#pragma once
#include "AudioMath.h"
#include "FFT.h"
#include "FFTData.h"

#include <functional>
#include <vector>


/**
 * some utils for modules, some for testing...
 */
class FFTUtils
{
public:
    class Stats {
    public:
        double largestPhaseJump = 0;
    };
    using Generator = std::function<double()>;
    static void getStats(Stats&, const FFTDataCpx& a, const FFTDataCpx& b, const FFTDataCpx& c);
    static std::vector<FFTDataRealPtr> generateData(int numSamples, int frameSize, Generator generator);
    static std::vector<FFTDataCpxPtr> generateFFTs(int numSamples, int frameSize, Generator generator);

    static Generator makeSinGenerator(double periodInSamples, double initialPhase);

    /**
     * xxx Discontinuity 0..1, where 1 is two pi
     * NO - discontinuity is in radians
     */
    static Generator makeSinGeneratorPhaseJump(double periodInSamples, double initialPhase, int delay, double discontinuityRadians);

};

/**
 * this helper should know everything about phase that goes from 0 .. 2pi
 */
class PhaseAngleUtil
{
public:
    static bool isNormalized(double phase) {
        return phase > -AudioMath::Pi && phase < AudioMath::Pi;
    }
    static double normalize(double phase) {
        while(phase <= -AudioMath::Pi) {
            phase += AudioMath::_2Pi;
        }
        while(phase >= AudioMath::Pi) {
            phase -= AudioMath::_2Pi;
        }
        assert(isNormalized(phase));
        return phase;
    }
    static double distance(double to, double from) {
        return normalize(to - from);
    }

};
