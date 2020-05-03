
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
    FFTUtils::Generator gen = FFTUtils::makeSinGenerator(8, 0);
   
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

static void testGenerateSinInitPhase()
{
    // start at 90 degrees
    FFTUtils::Generator gen = FFTUtils::makeSinGenerator(8, AudioMath::Pi_2);

    double x = gen();
    assertEQ(x, 1);
    x = gen();
    assertClose(x, 1.f / std::sqrt(2.f), .001);
}
   
static void testGenerateSinJump()
{
    const int sampleToJumpAt = 3;
    FFTUtils::Generator gen = FFTUtils::makeSinGeneratorPhaseJump(8, 0, sampleToJumpAt, AudioMath::Pi);

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

static void testGenerateSinJumpInitialPhase()
{
    const int sampleToJumpAt = 2;
    FFTUtils::Generator gen = FFTUtils::makeSinGeneratorPhaseJump(8, AudioMath::Pi_2, sampleToJumpAt, AudioMath::Pi);

    // sfirst two like sin
    double x = gen();
    assertEQ(x, 1);
    x = gen();
    assertEQ(x, 1.f / std::sqrt(2.f));

    // and jump 180 degrees
    x = gen();
    assertClose(x, 0, .001);

    x = gen();
    assertClose(x, 1.f / std::sqrt(2.f), .001);
  
}

static void testAnalyzePureSin()
{
    printf("\ntestAnalyzePureSin\n");
    // fairly high freq sine wave at even freq.
    FFTUtils::Generator gen = FFTUtils::makeSinGenerator(32, 0);
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
           // printf("captured big bin %d mag=%f hase = %f\n", i, x.first, x.second);
        }
       // if (x.first > .01) printf("info bin %d mag=%f hase = %f\n", i, x.first, x.second);
    }
    assertEQ(biggestBin, sinBin);
}

static void testAnalyzePureSinInBetweenPitch()
{
    // fairly high freq sine wave at a freq in-betten even period
    double period = 512 / 16.5;
    FFTUtils::Generator gen = FFTUtils::makeSinGenerator(period, 0);
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
        }
    }

    assertEQ(biggestBin, sinBin);
}

/**
 * generates three frames of various test signal.
 */
static FFTUtils::Stats analyzeHelper(int sampleToJumpAt, double initialPhase, double periodInSamples)
{
    const bool jumpPhase = sampleToJumpAt > 0;
  //  const int sampleToJumpAt = 1024 + 512 / 2;  // in middle of third
    if (jumpPhase) printf("will jump at %d\n", sampleToJumpAt);
    FFTUtils::Generator gen = jumpPhase ?
        FFTUtils::makeSinGeneratorPhaseJump(periodInSamples, initialPhase, sampleToJumpAt, .5)
        : FFTUtils::makeSinGenerator(periodInSamples, initialPhase);

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

static void testAnalyzeNoJump()
{
    printf("\n*** test 2\n");
    FFTUtils::Stats stats = analyzeHelper(false, 0, 32);
    assertClose(stats.largestPhaseJump, 0, .001);

    printf("\n*** test 4\n");
    // no jump, shifted initial phase.
    stats = analyzeHelper(false, AudioMath::Pi, 32);
    assertClose(stats.largestPhaseJump, 0, .001);

    printf("\n*** test 5\n");
    // no jump, but shifted and "in between" freq
    stats = analyzeHelper(false, AudioMath::Pi, 79);
    assertGE(stats.largestPhaseJump, 0);
    assertLT(stats.largestPhaseJump, .03);

    printf("\n*** test 6\n");
    // no jump or shift, but "in between" freq
    stats = analyzeHelper(false, 0, 79);
    assertGE(stats.largestPhaseJump, 0);
    assertLT(stats.largestPhaseJump, .03);
}

/**
 * sampleOffsetForDiscontinuity is where in the third fft bit we will jump the phase
 */
static void testAnalyzeJump(int sampleOffsetForDiscontinuity)
{
    assert(sampleOffsetForDiscontinuity > 0);
    assert(sampleOffsetForDiscontinuity < 512);

    FFTUtils::Stats stats = analyzeHelper(1024 + sampleOffsetForDiscontinuity, 0, 32);

    // jump is max at start of bin. Almost no effect later.
    // I have no idea why it's coming out as .5
    double expectedJump = .5 * double(512 - sampleOffsetForDiscontinuity) / 512.0;
    assertClose(stats.largestPhaseJump, expectedJump, .01);

}

static void testAnalyzeJump()
{
    testAnalyzeJump(5);
    testAnalyzeJump(512 /2);
    testAnalyzeJump(512 / 3);
    testAnalyzeJump(512 / 4);
    testAnalyzeJump(400);
    testAnalyzeJump(508);
#if 1
    //just looking for number to see what's reasonable
    float minPhaseJumpExpected = .25f;
    float maxPhaseJumpExpected = .5f;



    printf("\n*** test 7\n");
    FFTUtils::Stats stats = analyzeHelper(1024 + 512 / 2, 0, 32);
    assertGT(stats.largestPhaseJump, minPhaseJumpExpected); 
    assertLT(stats.largestPhaseJump, maxPhaseJumpExpected);
    
    printf("\n*** test 8\n");
    stats = analyzeHelper(1024 + 512 / 3, 0, 32);
    assertGT(stats.largestPhaseJump, minPhaseJumpExpected);
    assertLT(stats.largestPhaseJump, maxPhaseJumpExpected);

    printf("\n*** test 9\n");
    stats = analyzeHelper(1024 + 512 / 4, 0, 32);
    assertGT(stats.largestPhaseJump, minPhaseJumpExpected);
    assertLT(stats.largestPhaseJump, maxPhaseJumpExpected);

    printf("\n*** test 10\n");
    // This one has  the discontinuity right at the start
    stats = analyzeHelper(1024 + 5, 0, 32);
    assertGT(stats.largestPhaseJump, minPhaseJumpExpected);
    assertLT(stats.largestPhaseJump, maxPhaseJumpExpected);
#endif
   
#if 0
    printf("\n*** test 11\n");
    // This one has the disc closer to the end

    // only .1 here
    stats = analyzeHelper(1024 + 400, 0, 32);
    assertGT(stats.largestPhaseJump, minPhaseJumpExpected);
    assertLT(stats.largestPhaseJump, maxPhaseJumpExpected);


    printf("\n*** test 11\n");
    // This one has the disc even closer to the end

    // only .1 here
    stats = analyzeHelper(1024 + 508, 0, 32);
    assertGT(stats.largestPhaseJump, minPhaseJumpExpected);
    assertLT(stats.largestPhaseJump, maxPhaseJumpExpected);
#endif
}


static void testPhaseAngleUtilIsNormalized()
{
    for (int i = -3; i < 3; ++i) {
        assert(PhaseAngleUtil::isNormalized(i));
    }
    assert(PhaseAngleUtil::isNormalized(AudioMath::Pi - .001));
    assert(!PhaseAngleUtil::isNormalized(AudioMath::Pi + .001));
    assert(!PhaseAngleUtil::isNormalized(7));
    assert(PhaseAngleUtil::isNormalized(-1));
}

static void testPhaseAngleUtilINormalize()
{
    assertClose(PhaseAngleUtil::normalize(AudioMath::_2Pi + .001), .001, .00001);

     for (int i = -3; i < 3; ++i) {
        assertEQ(PhaseAngleUtil::normalize(i), i);
    }
    assertEQ(PhaseAngleUtil::normalize(AudioMath::Pi - .001), AudioMath::Pi - .001);
    assertClose(PhaseAngleUtil::normalize(AudioMath::_2Pi + .001), .001, .00001);
    assertEQ(PhaseAngleUtil::normalize(7), 7 - AudioMath::_2Pi);
    assertEQ(PhaseAngleUtil::normalize(-4), AudioMath::_2Pi - 4);
}

static void testPhaseAngleUtilIDistance()
{
    assertEQ(PhaseAngleUtil::distance(1, 1), 0);
    assertEQ(PhaseAngleUtil::distance(100, 100), 0);
    assertEQ(PhaseAngleUtil::distance(1, -1), 2);
    assertEQ(PhaseAngleUtil::distance(-1, 1), -2);

    const double delta = .001;
    assertClose(PhaseAngleUtil::distance(AudioMath::Pi_2 - delta, AudioMath::Pi_2 + delta), -2 * delta, .00001);
    assertClose(PhaseAngleUtil::distance(AudioMath::Pi_2 + delta, AudioMath::Pi_2 - delta), 2 * delta, .00001);
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
    testGenerateSinInitPhase();
    testGenerateSinJump();
    testGenerateSinJumpInitialPhase();
    testAnalyzePureSin();
    testAnalyzePureSinInBetweenPitch();
    testAnalyzeNoJump();
    testAnalyzeJump();
}