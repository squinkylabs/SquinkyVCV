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
extern void perfTest();
extern void testFrequencyShifter();
extern void testStateVariable();
extern void testVocalAnimator();
extern void testObjectCache();
extern void testThread();
extern void testFFT();
extern void testRingBuffer();
extern void testManagedPool();
extern void testColoredNoise();
extern void testFFTCrossFader();
extern void testFinalLeaks();


#if !defined(_MSC_VER)

// temporary
void tryThread()
{
  pthread_t threadHandle = pthread_self();

        struct sched_param params;
        int policy=55;
        pthread_getschedparam(threadHandle, &policy, &params);
        printf("default was policy %d priority %d\n", policy, params.sched_priority);

        printf("FIFO=%d OTHER=%d\n", SCHED_FIFO, SCHED_OTHER);
        // first, let's go for max. only works if root
        policy = SCHED_FIFO;
        int maxFifo =  sched_get_priority_max(policy);

        params.sched_priority = maxFifo;
        int x = pthread_setschedparam (threadHandle, policy, &params);
        printf("set realtime ret %d. 0 is success. pri=%d\n", x, maxFifo);
        if (x != 0) {
            int policy = SCHED_OTHER;
            int maxOther =  sched_get_priority_max(policy);
             x = pthread_setschedparam (threadHandle, policy, &params);
            printf("set realtime ret %d. 0 is success. pri=%d\n", x, maxOther);

        }
        printf(" ESRCH: %d, EINVAL %d, EPERM %d ENOTSUP %d\n", ESRCH, EINVAL, EPERM, ENOTSUP );
}
#else
void tryThread()
{
}
#endif

int main(int argc, char ** argv)
{
    printf("--- test ----\n"); fflush(stdout);
    bool runPerf = false;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--perf") {
            runPerf = true;
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
    tryThread();

    testAudioMath();
    testRingBuffer();
    testManagedPool();
    testLookupTable();
    testObjectCache();
    testTestSignal();
    testBiquad();
    testSaw();
    
    testSinOscillator();
    testHilbert();
    testStateVariable();

    testFFT();
    testFFTCrossFader();
    testThread();
    
    // after testing all the components, test composites.
    testColoredNoise();
    testFrequencyShifter();
    testVocalAnimator();

    if (runPerf) {
        perfTest();
    }

    testFinalLeaks();

    // When we run inside Visual Studio, don't exit debugger immediately
#if defined(_MSC_VER)
    printf("Test passed. Press any key to continue...\n"); fflush(stdout);
    getchar();
#else
    printf("Tests passed.\n");
#endif
}