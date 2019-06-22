/**
  * Unit test entry point
  */

#include <stdio.h>
#include <assert.h>
#include <string>

extern void testBiquad();
extern void testTestSignal();
extern void testSaw();
extern void testLookupTable();
extern void testSinOscillator();
extern void testHilbert();
extern void testAudioMath();
extern void initPerf();
extern void perfTest();
extern void perfTest2();
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
extern void testMinBLEPVCO();
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
extern void testMidiPlayer();
extern void testMultiLag();
extern void testUtils();
extern void testIComposite();
extern void testMidiEditor();
extern void testMidiEditorNextPrev();
extern void testNoteScreenScale();
extern void testMidiEditorCCP();
extern void testMidiEditorSelection();
extern void testVec();
extern void testSeqClock();
extern void testMix8();
extern void testSlew4();
extern void testCommChannels();
extern void testLadder();
extern void testHighpassFilter();
extern void calQ();

#if 0
#include <sstream>
#include <iostream>
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

//#define _MIDIONLY

int main(int argc, char ** argv)
{
   // xx();
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
        perfTest2();
        perfTest();
        return 0;
    }

    testIComposite();
    testVec();
    testCommChannels();
    testMidiEvents();
    testFilteredIterator();
    testMidiDataModel();
    testMidiSong();
    testSeqClock();
    testMidiPlayer();
    testReplaceCommand();
    testUndoRedo();
    testMidiViewport();
 
    testMidiControllers();
    testMidiEditorSelection();
    testMidiEditorNextPrev();
    testMidiEditor();
    testMidiEditorCCP();
    testNoteScreenScale();

    testAudioMath();
    testRingBuffer();
    testGateTrigger();
    testManagedPool();
    testLookupTable();
    testObjectCache();
    testMultiLag();
    testSlew4();
    testMix8();


//#ifndef _MSC_VER
#if !defined(_MSC_VER) || !defined(_MIDIONLY)
    testTestSignal();
    testBiquad();
    testSaw();
    testClockMult();
    testDelay();
    testPoly();

    testSinOscillator();
    testMinBLEPVCO();
    testHilbert();
    testButterLookup();
   
    testVCO();
   
   // testSin();

   

    testFFT();
    testAnalyzer();
    testRateConversion();
    testUtils();
    //testLowpassFilter();
    testLadder();
    testHighpassFilter();
 
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
    printf("disabled lots of tests for MS\n");
#endif
    testFinalLeaks();

    // When we run inside Visual Studio, don't exit debugger immediately
#if defined(_MSC_VER_not)
    printf("Test passed. Press any key to continue...\n"); fflush(stdout);
    getchar();
#else
    printf("Tests passed.\n");
#endif
}

void sequencerHelp()
{

}