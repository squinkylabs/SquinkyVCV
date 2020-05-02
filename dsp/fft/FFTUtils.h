
#pragma once
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

    static Generator makeSinGenerator(double periodInSamples);

    /**
     * Discontinuity 0..1, where 1 is two pi
     */
    static Generator makeSinGeneratorPhaseJump(double periodInSamples, int delay, double discontinuity);

};