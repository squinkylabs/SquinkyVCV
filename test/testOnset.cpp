
#include "asserts.h"
#include "FFT.h"
#include "FFTData.h"

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

};

FFTUtils::Generator FFTUtils::makeSineGenerator(float periodInSamples)
{
    float phaseInc = 1.f / periodInSamples;
    Generator g =  [phaseInc]() {
        static float phase = 0;
        float ret =  std::sin(phase * 2.f * float(AudioMath::Pi));
        phase += phaseInc;
        return ret;
    };
    return g;
}

std::vector< FFTDataCpxPtr> FFTUtils::generateFFTs(int numSamples, int frameSize, std::function<float()> generator)
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

std::vector< FFTDataRealPtr> FFTUtils::generateData(int numSamples, int frameSize, std::function<float()> generator)
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
        float x = generator();
        buffer->set(index, x);
        ++index;
        if (index >= frameSize) {
            buffer.reset();
        }
    }
    return ret;
}

//***********************************************************************************************

class OnsetDetector
{
public:
    OnsetDetector(int size) : inputBuffer(size)
    {
    }

    void captureSample(float data)
    {
        if (inputIndex < inputBuffer.size()) {
           // inputBuffer[inputIndex++] = data;
            inputBuffer.set(inputIndex++, data);
        }

        if (inputIndex == inputBuffer.size())  {
            haveData = true;
            inputIndex = 0;
        }
    }

    /**
     * will get the results from last buffer, and clear it.
     * return.first is true if data from last frame was available
     * second it onset or not
     */
    std::pair<bool, bool> wasOnset()
    {
        bool hadData = haveData;
        haveData = false;
        return std::make_pair(hadData, false);
    }
private:
    FFTDataReal inputBuffer;
    int inputIndex = 0;
    bool haveData = false;
};

static void test0()
{
    OnsetDetector o(512);
    assertEQ(o.wasOnset().first, false);
    o.captureSample(0);
    assertEQ(o.wasOnset().first, false);
}

static void test1()
{
    OnsetDetector o(512);
    for (int i = 0; i < 511; ++i) {
        o.captureSample(0);
        assertEQ(o.wasOnset().first, false);
    }
   
    o.captureSample(0);
    assertEQ(o.wasOnset().first, true);
    assertEQ(o.wasOnset().first, false);
}

static void testGenerateData()
{
    auto result = FFTUtils::generateData(1024, 512, []() { return 0.f; });
    assertEQ(result.size(), 2);
}


static void testGenerateData2()
{
    auto result = FFTUtils::generateData(1, 512, []() { return 1.f; });
    assertEQ(result.size(), 1);
    FFTDataRealPtr buffer = result[0];
    assertEQ(buffer->get(0), 1.f);
    assertEQ(buffer->get(1), 0.f);
}

static void testGenerateFFT()
{
    auto result = FFTUtils::generateFFTs(1024, 512, []() { return 1.f; });
    assertEQ(result.size(), 2);
}

static void testGenerateSin()
{
    FFTUtils::Generator gen = FFTUtils::makeSineGenerator(8);
    float x = gen();
    assertEQ(x, 0);
    x = gen();
    assertEQ(x, 1.f / std::sqrt(2.f));
    x = gen();
    assertEQ(x, 1.f);
    x = gen();
    assertEQ(x, 1.f / std::sqrt(2.f));
    x = gen();
    assertClose(x, 0, .0001);
    x = gen();
    assertClose(x, -1.f / std::sqrt(2.f), .0001);
    x = gen();
    assertEQ(x, -1.f );
    x = gen();
    assertClose(x, -1.f / std::sqrt(2.f), .0001);

    x = gen();
    assertClose(x, 0, .0001);

}
   
/* next test. need to make three mag phase frames. analyzie
 */

void testOnset()
{
    test0();
    test1();
    testGenerateData();
    testGenerateData2();
    testGenerateFFT();
    testGenerateSin();
}