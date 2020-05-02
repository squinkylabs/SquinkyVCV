
#include "asserts.h"

#include "FFTUtils.h"

// ***********************************************************************************************

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
    FFTUtils::Generator gen = FFTUtils::makeSinGenerator(8);
   
    for (int i = 0; i < 10; ++i) {
        double x = gen();
        assertEQ(x, 0);
        x = gen();
        assertClose(x, 1.f / std::sqrt(2.f), .001);
        x = gen();
        assertEQ(x, 1.f);
        x = gen();
        assertClose(x, 1.f / std::sqrt(2.f), .001);
        x = gen();
        assertClose(x, 0, .0001);
        x = gen();
        assertClose(x, -1.f / std::sqrt(2.f), .0001);
        x = gen();
        assertEQ(x, -1.f);
        x = gen();
        assertClose(x, -1.f / std::sqrt(2.f), .0001);
    }
}
   
static void testGenerateSinJump()
{
    const int sampleToJumpAt = 3;
    FFTUtils::Generator gen = FFTUtils::makeSinGeneratorPhaseJump(8, sampleToJumpAt, AudioMath::Pi);

    // sfirst three like sin
    double x = gen();
    assertEQ(x, 0);
    x = gen();
    assertEQ(x, 1.f / std::sqrt(2.f));
    x = gen();
    assertEQ(x, 1.f);

    // and jump 180 degrees
    x = gen();
    assertClose(x, -1.f / std::sqrt(2.f), .0001);
}

static void testAnalyzePureSin()
{
    printf("\ntestAnalyzePureSin\n");
    // fairly high freq sine wave at even freq.
    FFTUtils::Generator gen = FFTUtils::makeSinGenerator(32);
    auto result = FFTUtils::generateFFTs(512 * 3, 512, gen);
    assertEQ(result.size(), 3);

    int sinBin = 512 / 32;

    result[0]->toPolar();
    float biggest = 0;
    int biggestBin = -1;
    for (int i = 0; i < 512; ++i) {
        auto x = result[0]->getMagAndPhase(i);
        if (x.first > biggest) {
            biggest = x.first;
            biggestBin = i;
            printf("captured big bin %d mag=%f hase = %f\n", i, x.first, x.second);
        }
        if (x.first > .01) printf("info bin %d mag=%f hase = %f\n", i, x.first, x.second);
    }
    assertEQ(biggestBin, sinBin);
}

static void testAnalyzePureSinInBetweenPitch()
{
    printf("\ntestAnalyzePureSinInBetweenPitch\n");
    // fairly high freq sine wave at a freq in-betten even period
    double period = 512 / 16.5;
    FFTUtils::Generator gen = FFTUtils::makeSinGenerator(period);
    auto result = FFTUtils::generateFFTs(512 * 3, 512, gen);
    assertEQ(result.size(), 3);

    int sinBin = 512 / 32;

    result[0]->toPolar();
    float biggest = 0;
    int biggestBin = -1;
    for (int i = 0; i < 512; ++i) {
        auto x = result[0]->getMagAndPhase(i);
        if (x.first > biggest) {
            biggest = x.first;
            biggestBin = i;
            //printf("captured big bin %d mag=%f hase = %f\n", i, x.first, x.second);
        }
      //  if (x.first > .01) printf("info bin %d mag=%f hase = %f\n", i, x.first, x.second);
    }

    assertEQ(biggestBin, sinBin);
}
static FFTUtils::Stats analyzeHelper(bool jumpPhase) {

    const int sampleToJumpAt = 1024 + 512 / 2;  // in middle of third
    if (jumpPhase) printf("will jump at %d\n", sampleToJumpAt);
    FFTUtils::Generator gen = jumpPhase ?
        FFTUtils::makeSinGeneratorPhaseJump(32, sampleToJumpAt, .5)
        : FFTUtils::makeSinGenerator(32);

    printf("about to gen fft\n");
    auto result = FFTUtils::generateFFTs(512 * 3, 512, gen);
    assertEQ(result.size(), 3);
    printf("generated\n");
    for (auto frame : result) {
        frame->toPolar();
    }

    FFTUtils::Stats stats;
    FFTUtils::getStats(stats, *result[0], *result[1], *result[2]);
    return stats;
}

static void testAnalyze2()
{
    printf("*** a2\n");
     FFTUtils::Stats stats = analyzeHelper(false); 
     assertClose(stats.largestPhaseJump, 0, .001);
}

static void testAnalyze3()
{
    printf("*** a3\n");
     FFTUtils::Stats stats = analyzeHelper(true); 


     assertClose(stats.largestPhaseJump, AudioMath::Pi, .001);
}

static void testPhaseAngleUtilIsNormalized()
{
    for (int i = 0; i < 7; ++i) {
        assert(PhaseAngleUtil::isNormalized(i));
    }
    assert(PhaseAngleUtil::isNormalized(AudioMath::_2Pi - .001));
    assert(!PhaseAngleUtil::isNormalized(AudioMath::_2Pi + .001));
    assert(!PhaseAngleUtil::isNormalized(7));
    assert(!PhaseAngleUtil::isNormalized(-1));

}

static void testPhaseAngleUtilINormalize()
{
    
    assertClose(PhaseAngleUtil::normalize(AudioMath::_2Pi + .001), .001, .00001);

     for (int i = 0; i < 7; ++i) {
        assertEQ(PhaseAngleUtil::normalize(i), i);
    }
    assertEQ(PhaseAngleUtil::normalize(AudioMath::_2Pi - .001), AudioMath::_2Pi - .001);
    assertClose(PhaseAngleUtil::normalize(AudioMath::_2Pi + .001), .001, .00001);
    assertEQ(PhaseAngleUtil::normalize(7), 7 - AudioMath::_2Pi);
    assertEQ(PhaseAngleUtil::normalize(-1), AudioMath::_2Pi - 1);

}

static void testPhaseAngleUtilIDistance()
{
    assertEQ(PhaseAngleUtil::distance(1, 1), 0);
    assertEQ(PhaseAngleUtil::distance(100, 100), 0);
    assertEQ(PhaseAngleUtil::distance(1, -1), 2);
    assertEQ(PhaseAngleUtil::distance(-1, 1), AudioMath::_2Pi -2);
}

void testOnset()
{
    test0();
    test1();
    testPhaseAngleUtilIsNormalized();
    testPhaseAngleUtilINormalize();
    testPhaseAngleUtilIDistance();

    testGenerateData();
    testGenerateData2();
    testGenerateFFT();
    testGenerateSin();
    testGenerateSinJump();
    testAnalyzePureSin();
    testAnalyzePureSinInBetweenPitch();
    testAnalyze2();
    testAnalyze3();
}