/**
  * Unit test entry point
  */

#include <assert.h>
#include <stdio.h>

#include <string>

extern void testMidiPlayer2();
extern void testMidiPlayer4();
extern void testBiquad();
extern void simd_testBiquad();
extern void testTestSignal();
extern void testSaw();
extern void testLookupTable();
extern void testSinOscillator();
extern void testHilbert();
extern void testAudioMath();
extern void initPerf();
extern void perfTest();
extern void perfTest2();
extern void perfTest3();
extern void testFrequencyShifter();
extern void testStateVariable();
extern void testVocalAnimator();
extern void testObjectCache();
extern void testThread(bool exended);
extern void testFFT();
extern void testRingBuffer();
extern void testManagedPool();
extern void testColoredNoise();
extern void testFFTCrossFader();
extern void testFinalLeaks();
extern void testClockMult();
extern void testTremolo();
extern void testGateTrigger();
extern void testAnalyzer();
extern void testFilter();
extern void testStochasticGrammar();
extern void testGMR();
extern void testLowpassFilter();
extern void testPoly();
extern void testVCO();
extern void testFilterDesign();
extern void testVCOAlias();
extern void testSin();
extern void testRateConversion();
extern void testDelay();
extern void testSpline(bool emit);
extern void testButterLookup();
extern void testMidiDataModel();
extern void testMidiSong();
extern void testReplaceCommand();
extern void testUndoRedo();
extern void testMidiViewport();
extern void testFilteredIterator();
extern void testMidiEvents();
extern void testMidiControllers();
extern void testMultiLag();
extern void testMultiLag2();
extern void testUtils();
extern void testIComposite();
extern void testMidiEditor();
extern void testMidiEditorNextPrev();
extern void testNoteScreenScale();
extern void testMidiEditorCCP();
extern void testMidiEditorSelection();
extern void testSeqComposite();
extern void testSeqComposite4();
extern void testVec();
extern void testSeqClock();
extern void testMix8();
extern void testMix4();
extern void testMixHelper();
extern void testStereoMix();
extern void testSlew4();
extern void testCommChannels();
extern void testLadder();
extern void testHighpassFilter();
extern void calQ();
extern void testDrumTrigger();
extern void testAudition();
extern void testStepRecordInput();
extern void testMidiFile();
extern void testNewSongDataDataCommand();
extern void testScale();
extern void testTriad();
extern void testMidiSelectionModel();
extern void testMidiTrackPlayer();
extern void testSuper();
extern void testEditCommands4();
extern void testChaos();
extern void testOnset();
extern void testOnset2();
extern void testWVCO();
extern void testSub();
extern void testSimd();
extern void testSimdLookup();
extern void testSimpleQuantizer();
extern void testDC();
extern void testSines();
extern void testBasic();
extern void testFilterComposites();
extern void testClockRecovery();
extern void testCompCurves();
extern void testSqStream();
extern void testOscSmoother();
extern void testx();
extern void testx2();
extern void testx3();
extern void testx4();
extern void testx5();
extern void testx6();
extern void testx7();
extern void testADSR();
extern void testADSRSampler();
extern void testWavThread();
extern void testACDetector();
extern void testCmprsr();
extern void testCompressor();
extern void testCompressorII();
extern void testCompressorParamHolder();
extern void testStreamer();
extern void testSampComposite();
extern void testFlac();
extern void testHeadingTracker();
extern void do_gc();

#if 0
#include <iostream>
#include <sstream>
static void xx()
{
    std::stringstream s;
    for (int i = 0; i < 10; ++i) {
        s << "A" << i << "_PARAM," << std::endl;
    }
    for (int i = 0; i < 10; ++i) {
        s << "B" << i << "_PARAM," << std::endl;
    }


    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            s << "A" << i << "B" << j << "_PARAM," << std::endl;
        }
    }
    std::cout << s.str();
}
#endif

// #define _MIDIONLY

// This will turn on more Microsoft malloc debug
#if 0
static void setupMem()
{
    // default is 1
    int f = _CrtSetDbgFlag(
        _CRTDBG_ALLOC_MEM_DF |
        _CRTDBG_DELAY_FREE_MEM_DF |
        _CRTDBG_CHECK_CRT_DF |
        _CRTDBG_LEAK_CHECK_DF 
   );
}
#endif

int main(int argc, char** argv) {
    // setupMem();

    bool runPerf = false;
    bool extended = false;
    bool runShaperGen = false;
    bool cq = false;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--ext") {
            extended = true;
        } else if (arg == "--perf") {
            runPerf = true;
        } else if (arg == "--shaper") {
            runShaperGen = true;
        } else if (arg == "--calQ") {
            cq = true;
        } else {
            printf("%s is not a valid command line argument\n", arg.c_str());
        }
    }
    // do_gc();
#ifdef _PERF
    runPerf = true;
#ifndef NDEBUG
#error asserts should be off for perf test
#endif
#endif
    // While this code may work in 32 bit applications, it's not tested for that.
    // Want to be sure we are testing the case we care about.
    assert(sizeof(size_t) == 8);

    if (runShaperGen) {
        testSpline(true);
        return 0;
    }

    if (cq) {
        calQ();
        return 0;
    }

    if (runPerf) {
        initPerf();
        perfTest3();
        perfTest2();

        perfTest();
        return 0;
    }

    testCompCurves();

    // mode these up where while we work...
    testMultiLag2();
    testCmprsr();
    testCompressorII();
    testCompressorParamHolder();
    testCompressor();

    testSimpleQuantizer();
    testFilterComposites();

    testStreamer();
    testHeadingTracker();
    testx();
    testx2();
    testx3();
    testx4();
    testx5();
    testx6();
    testx7();

    testSampComposite();

    testFlac();
    testADSRSampler();
   
    testADSR();

    testWavThread();
    testIComposite();
    testClockRecovery();
    testCompCurves();

    testBasic();
    testSines();
    testDC();
    testSimd();
    testSimdLookup();

    testOscSmoother();

    testAudioMath();
    testRingBuffer();
    testGateTrigger();
    testSimpleQuantizer();
    testOnset();
    testOnset2();

    testBiquad();
    testACDetector();

    testSqStream();
    testSub();
    simd_testBiquad();
    testWVCO();

    testSimpleQuantizer();

    testVec();
    testCommChannels();
    testMidiEvents();
    testFilteredIterator();
    testMidiDataModel();
    testMidiSelectionModel();
    testChaos();
    testMidiSong();
    testSeqClock();
    testMidiTrackPlayer();
    testMidiPlayer2();
    testMidiPlayer4();
    testMidiViewport();
    testScale();
    testTriad();
    testReplaceCommand();
    testNewSongDataDataCommand();
    testUndoRedo();

    testMidiFile();
    testMidiControllers();
    testMidiEditorSelection();
    testMidiEditorNextPrev();
    testMidiEditor();
    testMidiEditorCCP();
    testNoteScreenScale();
    testSeqComposite();
    testSeqComposite4();
    testAudition();

    testDrumTrigger();
    testStepRecordInput();

    testManagedPool();
    testLookupTable();
    testObjectCache();
    testEditCommands4();
    testMultiLag();
    testSlew4();
    testMixHelper();
    testStereoMix();
    testMix4();
    testMix8();

#if !defined(_MIDIONLY)
    testTestSignal();

    testSaw();
    testClockMult();
    testDelay();
    testPoly();
    testSinOscillator();
    testHilbert();
    testButterLookup();
    testVCO();
    // testSin();
    testFFT();
    testAnalyzer();
    testRateConversion();
    testUtils();
    testLowpassFilter();
    testLadder();
    testHighpassFilter();
    testSuper();

#if 0
    printf("skipping lots of tests\n");
#else
    testSpline(false);
    testStateVariable();
    testFFTCrossFader();
    if (extended) {
        testThread(extended);
    }

    testFilter();

    testStochasticGrammar();
    testGMR();

    // after testing all the components, test composites.
    testTremolo();
    testColoredNoise();

    testFrequencyShifter();
    testVocalAnimator();
#endif

    testFilterDesign();
#else
    printf("disabled lots of tests for MS (or MIDI ONLY)\n");
#endif
    testFinalLeaks();

    // When we run inside Visual Studio, don't exit debugger immediately
#if defined(_MSC_VER_not)
    printf("Test passed. Press any key to continue...\n");
    fflush(stdout);
    getchar();
#else
    printf("Tests passed.\n");
#endif
}

void sequencerHelp() {
}