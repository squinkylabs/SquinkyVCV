
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

    };
    using Generator = std::function<float()>;
    static void getStats(Stats&, const FFTDataCpx& a, const FFTDataCpx& b, const FFTDataCpx& c);
    static std::vector<FFTDataRealPtr> generateData(int numSamples, int frameSize, Generator generator);
    static std::vector<FFTDataCpxPtr> generateFFTs(int numSamples, int frameSize, Generator generator);

    static Generator makeSineGenerator(float periodInSamples);

    /**
     * Discontinuity 0..1, where 1 is two pi
     */
    static Generator makeSineGeneratorPhaseJump(float periodInSamples, int delay, float discontinuity);

};