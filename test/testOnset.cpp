
#include "asserts.h"

#include "FFTUtils.h"
#include "OnsetDetector.h"
#include "SqWaveFile.h"
#include "TestGenerators.h"

// ***********************************************************************************************

#if 0
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
#endif

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
    FFTUtils::Generator gen = TestGenerators::makeSinGenerator(8, 0);
   
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
    FFTUtils::Generator gen = TestGenerators::makeSinGenerator(8, AudioMath::Pi_2);

    double x = gen();
    assertEQ(x, 1);
    x = gen();
    assertClose(x, 1.f / std::sqrt(2.f), .001);
}
   
static void testGenerateSinJump()
{
    const int sampleToJumpAt = 3;
    FFTUtils::Generator gen = TestGenerators::makeSinGeneratorPhaseJump(8, 0, sampleToJumpAt, AudioMath::Pi);

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
    FFTUtils::Generator gen = TestGenerators::makeSinGeneratorPhaseJump(8, AudioMath::Pi_2, sampleToJumpAt, AudioMath::Pi);

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
    FFTUtils::Generator gen = TestGenerators::makeSinGenerator(32, 0);
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
    FFTUtils::Generator gen = TestGenerators::makeSinGenerator(period, 0);
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
  //  if (jumpPhase) printf("will jump at %d\n", sampleToJumpAt);
    TestGenerators::Generator gen = jumpPhase ?
        TestGenerators::makeSinGeneratorPhaseJump(periodInSamples, initialPhase, sampleToJumpAt, .5)
        : TestGenerators::makeSinGenerator(periodInSamples, initialPhase);

    auto result = FFTUtils::generateFFTs(512 * 3, 512, gen);
    assertEQ(result.size(), 3);
    for (auto frame : result) {
        frame->toPolar();
    }

    FFTUtils::Stats stats;
    FFTUtils::getStats(stats, *result[0], *result[1], *result[2]);
    return stats;
}


// #define _USE2
// let's use the new sin generator here, becuase:
//  a) the old one is a saw
//  b) this will make it easier to find out what's going on in testOnset2

// orig way at 10.3 bin9: 
//          bin 9 mag 0.105684 jump=0.170600, ph = -3.031146, -1.089607, 0.681333
//                  .58 . .905.637
// new calc bin 9 mag 0.105684 jump=0.170600, ph = -3.031146, -1.089607, 0.681333
//           total = .58, .905, .63
static void testAnalyzeNoJump2b()
{
    printf("\n\n********************* testAnalyzeNoJump2b\n");
    // testAnalyzeNoJump(512.0 / 10.3);

    // bad results
    //double period = 512 / 10.3;

    // perfect at 512 / 10.0
    // at 10.05 bin 9 is .02 mag, jump = .02
    // at 10.1, bin 9 is .-5 mag juimp = .07
    // at 10.3  bin 9 is  .1 mag, jump = .17
    // eed
    // 10.02 is ok

    // let's look at the main (10) band over 20 frames
    // at 10.0 it's perfect
    // at 10.02 jump is .0000nnn
    // 10.05 .000nn
    // 10.1 .00nnn
    // 10.2 .0n
    // 10.025 .0n
    // 10.3 .05
    // 10.4 0.n
    // 10.5 perfect (??) but mag .3

    // conclusion - bin10 looks pretty good. need to look more at bin 9.
    double period = 512 / 10.6;
    double freq = 1.0 / period;
    freq *= AudioMath::_2Pi;

    TestGenerators::Generator gen = TestGenerators::makeSinGenerator(freq);

    const int numFrames = 20;

    auto result = FFTUtils::generateFFTs(512 * numFrames, 512, gen);
    assertEQ(result.size(), numFrames);
    FFTUtils::Stats stats;

#if defined _USE2
    printf("\nfirst analyze frame:\n");
    FFTUtils::getStats2(stats, *result[0], *result[1], *result[2]);
     printf("\nsecond analyze frame:\n");
    FFTUtils::getStats2(stats, *result[1], *result[2], *result[3]);
     printf("\nthird analyze frame:\n");
    FFTUtils::getStats2(stats, *result[2], *result[3], *result[4]);
#else
    for (auto frame : result) {
        frame->toPolar();
    }
    for (int i = 0; i < numFrames - 2; ++i) {
        printf("\nframe %d analysis: \n", i);
        FFTUtils::getStats(stats, *result[0+i], *result[1+i], *result[2+i]);
    }

#endif

   
   
    assert(false);
}


static void testAnalyzeNoJump(double periodInSamples)
{
    FFTUtils::Stats stats = analyzeHelper(false, 0, 32);
    assertClose(stats.averagePhaseJump, 0, .001);

    // no jump, shifted initial phase.
    stats = analyzeHelper(false, AudioMath::Pi, 32);
    assertClose(stats.averagePhaseJump, 0, .001);

    // no jump, but shifted and "in between" freq
    stats = analyzeHelper(false, AudioMath::Pi, 79);
    assertGE(stats.averagePhaseJump, 0);
    assertLT(stats.averagePhaseJump, .03);

    // no jump or shift, but "in between" freq
    stats = analyzeHelper(false, 0, 79);
    assertGE(stats.averagePhaseJump, 0);
    assertLT(stats.averagePhaseJump, .03);
}

static void testAnalyzeNoJump()
{
    testAnalyzeNoJump(32);
}

static void testAnalyzeNoJump2()
{
    testAnalyzeNoJump(512.0 / 10.3);
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
    // .5 is the max phase shift, becuase that's what we pass to the sin generator
    double expectedJump = .5 * double(512 - sampleOffsetForDiscontinuity) / 512.0;
    assertClose(stats.averagePhaseJump, expectedJump, .01);
}

static void testAnalyzeJump()
{
    testAnalyzeJump(5);
    testAnalyzeJump(512 /2);
    testAnalyzeJump(512 / 3);
    testAnalyzeJump(512 / 4);
    testAnalyzeJump(400);
    testAnalyzeJump(508);
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

static void testWaveFile()
{
    SqWaveFile wave;
    bool b = wave.load("multi-note.wav");
    assert(b);
    int size = wave.size();
    assert(size > 100);
    for (int i = 0; i < size; ++i) {
        float x = wave.getAt(i);
        assert(x <= 1);
        assert(x >= -1);
    }
}

static void testWaveFile2()
{
    SqWaveFile wave;
    bool b = wave.loadTest(SqWaveFile::TestFiles::SingleShortNote);
    assert(b);
    int size = wave.size();
    assert(size > 0);

    b = wave.loadTest(SqWaveFile::TestFiles::MultiNote);
    assert(b);
    size = wave.size();
    assert(size > 0);
}

class TestOnsetDetector
{
public:
    static void test1()
    {
        OnsetDetector o;
        assertEQ(o.curFrame, 0);
        assertEQ(o.nextFrame(), 1);
        assertEQ(o.prevFrame(), 2);
        assertEQ(o.prevPrevFrame(), 1);

        o.curFrame = 1;
        assertEQ(o.nextFrame(), 2);
        assertEQ(o.prevFrame(), 0);
        assertEQ(o.prevPrevFrame(), 2);
        
        o.curFrame = 2;
        assertEQ(o.nextFrame(), 0);
        assertEQ(o.prevFrame(), 1);
        assertEQ(o.prevPrevFrame(), 0);
    }
};

 static void testOnsetDetector()
 {
    TestOnsetDetector::test1();
 }

#if 1
 static void testing()
 {
     const double pi2 = AudioMath::_2Pi;
     printf("pi = %.2f 2pi=%.2f\n", AudioMath::Pi, pi2);
     // bin 9 mag 0.100510 jump = 0.170600, ph = -0.570886, 1.200053, -3.141593
     // bin 10 mag 0.430429 jump = 0.036878, ph = -0.615644, 1.244535, 3.141593
    double ph0 = -0.570886;
    double ph1 = 1.200053;
    
    double ph2 = -3.141593;
    double ph2p = ph2 + pi2;

    printf("ph0 ph1 ph2: %.2f, %.2f, %.2f ->  %.2f, %.2f ->%.2f\n", ph0, ph1, ph2, ph1 - ph0, ph2 - ph1, ph2 - 2* ph1 + ph0);
    printf("ph0 ph1 ph2p: %.2f, %.2f, %.2f ->  %.2f, %.2f ->%.2f\n", ph0, ph1, ph2p, ph1 - ph0, ph2p - ph1, ph2p - 2 * ph1 + ph0);

    ph0 = -ph0;
    ph1 = -ph1;
    ph2 = -ph2;
    printf("neg ph0 ph1 ph2: %.2f, %.2f, %.2f ->  %.2f, %.2f ->%.2f\n", ph0, ph1, ph2, ph1 - ph0, ph2 - ph1, ph2 - 2 * ph1 + ph0);
    printf("neg ph0 ph1 ph2: %.2f, %.2f, %.2f ->  %.2f, %.2f ->%.2f\n", ph0, ph1, ph2, ph1 - ph0, ph2 - ph1, ph2 - 2 * ph1 + ph0);
   // assert(false);

 }
#endif

void testOnset()
{
#if 0
  //  test0();
  //  test1();
    testing();
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
    testAnalyzeNoJump2();
    testAnalyzeNoJump2b();
    assert(false);

    testAnalyzeJump();

    testWaveFile();
    testWaveFile2();
    testOnsetDetector();
#endif
}