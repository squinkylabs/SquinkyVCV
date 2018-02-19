#pragma once

template <typename T>
class MeasureTime
{

public:
    static timespec diff(timespec start, timespec end)
    {
        timespec temp;
        if ((end.tv_nsec - start.tv_nsec) < 0) {
            temp.tv_sec = end.tv_sec - start.tv_sec - 1;
            temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
        } else {
            temp.tv_sec = end.tv_sec - start.tv_sec;
            temp.tv_nsec = end.tv_nsec - start.tv_nsec;
        }
        return temp;
    }

    // run test iterators time, return total seconds

    static double measureTimeSub(std::function<T()> func, int64_t iterations)
    {
        struct timespec t0;
        int x = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t0);

        for (int64_t i = 0; i < iterations; ++i) {
            const T x = func();
            TestBuffers<T>::put(x);
        }

        struct timespec t1;
        x = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);

        struct timespec elapsed = diff(t0, t1);

        double seconds = (double) elapsed.tv_sec + (elapsed.tv_nsec / (1000.0 * 1000.0 * 1000.0));
        // printf("sec=%f tv_sec=%lld tv_ns=%d\n", seconds, elapsed.tv_sec, elapsed.tv_nsec);
        return seconds;
    }

  
    static void run(const char * name, std::function<T()> func, float minTime)
    {
        int64_t iterations;
        bool done = false;
        for (iterations = 100; !done; iterations *= 2) {
            double elapsed = measureTimeSub(func, iterations);
            if (elapsed >= minTime) {
                double itersPerSec = iterations / elapsed;
                double full = 44100;
                double percent = full * 100 / itersPerSec;
                printf("\nmeasure %s over time %f\n", name, minTime);
                printf("did %lld iterations in %f seconds\n", iterations, elapsed);
                printf("that's %f per sec\n", itersPerSec);
                printf("percent CPU usage: %f\n", percent);
                printf("best case instances: %f\n", 100 / percent);
                printf("quoata used per 1 percent : %f\n", percent * 100);
                fflush(stdout);
                done = true;
            }
        }
    }
};

/**
 * Simple producer / consumer for test data.
 * Serves up a pre-calculated list of random numbers.
 */
template <typename T>
class TestBuffers
{
public:
    static const size_t size = 60000;
    static void put(T x)
    {
        destData[destIndex++] = x;
        if (destIndex >= size) {
            destIndex = 0;
        }
    }
    static T get()
    {
        T ret = sourceData[sourceIndex++];
        if (sourceIndex >= size) {
            sourceIndex = 0;
        }
        return ret;
    }
    //
    TestBuffers()
    {
        for (int i = 0; i < size; ++i) {
            source[i] = (float) rand() / (float) RAND_MAX;
        }
    }
private:
    static size_t sourceIndex;
    static size_t destIndex;
    static T sourceData[size];
    static T destData[size];
};


template <typename T>
T TestBuffers<T>::sourceData[size];

template <typename T>
T TestBuffers<T>::destData[size];

template <typename T>
size_t TestBuffers<T>::sourceIndex = 0;

template <typename T>
size_t TestBuffers<T>::destIndex = 512;


/**
 * Simple timer implementation for running inside Visual Studio
 *
 * Not needed Linux, where clock_gettime is available
 */
#ifdef _MSC_VER
#include <windows.h>
struct timespec__
{
    time_t tv_sec;
    long tv_nsec;
};

int clock_gettime(int id, struct timespec* ts)
{
    const int ms = ::timeGetTime();
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000 * 1000;

    return 0;
}
const int CLOCK_THREAD_CPUTIME_ID = 0;
#endif


