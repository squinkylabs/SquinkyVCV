#pragma once
/**
 * @class SqTime
 * A simple high-res timer. Returns current seconds
 */


/**
 * Windows version is based on QueryPerformanceFrequency
 * Typically this is accurate to 1/3 microsecond. Can be made more
 * accurate by tinkering with your bios
 */
#if defined(_MSC_VER) || defined(ARCH_WIN)
#include <Windows.h>
class SqTime
{
public:
    static double seconds()
    {
        LARGE_INTEGER t;
        if (frequency == 0) {

            QueryPerformanceFrequency(&t);
            frequency = double(t.QuadPart);
        }

        QueryPerformanceCounter(&t);
        int64_t n = t.QuadPart;
        return double(n) / frequency;
    }
private:
    static double frequency;
};
double SqTime::frequency = 0;
#endif


// non-windows will probably use this
#if 0
#include <windows.h>

int clock_gettime(int id, struct timespec* ts)
const int CLOCK_THREAD_CPUTIME_ID = 0;
#endif