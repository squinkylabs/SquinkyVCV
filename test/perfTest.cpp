#include <functional>
#include <time.h>
#include <cmath>
#include <limits>

static float next()
{
    const int size = 60000;
    static float data[size];
    static bool init = false;
    static int index = 0;
    if (!init) {
        for (int i = 0; i < size; ++i) {
            data[i] = (float) rand() / (float) RAND_MAX;
        }
        init = true;
    }
    float ret = data[index++];
    if (index >= size) {
        index = 0;
    }
    return ret;
}
const int size = 60000;
float data[size];

static void put(float f)
{

    static float data[size];

    static int index = 0;

    data[index++] = f;
    if (index >= size) {
        index = 0;
    }

}

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

timespec diff(timespec start, timespec end)
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
double measureTimeSub(std::function<float()> func, int64_t iterations)
{
    struct timespec t0;
    int x = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t0);

    for (int64_t i = 0; i < iterations; ++i) {
        const float x  = func();
        put(x);
    }

    struct timespec t1;
    x = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);

    struct timespec elapsed = diff(t0, t1);

    double seconds = (double) elapsed.tv_sec + (elapsed.tv_nsec / (1000.0 * 1000.0 * 1000.0));
   // printf("sec=%f tv_sec=%lld tv_ns=%d\n", seconds, elapsed.tv_sec, elapsed.tv_nsec);
    return seconds;
}

void measureTime(const char * name, std::function<float()> func, float minTime)
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




static void test1()
{
    double d = .1;
    srand(57);
    const double scale = 1.0 / RAND_MAX;
    next();
    int64_t calls = 0;

    measureTime("test1 null", [&d, scale]() {
        return next();
     }, 1);

#if 0
    measureTime("test1 null", [&d, scale]() {
        return next();
        }, 10);
#endif

    measureTime("test1 sin", [&d, scale, &calls]() {
        float x = std::sin(next());
        ++calls;
        return x;
        }, 1);

    measureTime("test1 sinx2", [&d, scale, &calls]() {
        float x = std::sin(next());
        x = std::sin(x);
        ++calls;
        return x;
        }, 1);

    printf("total sin calls = %lld\n", calls);
#if 0
    measureTime("test1 tanh", [&d, scale]() {
        double x = std::tanh(next());
        }, 10);

    measureTime("test1 null again", [&d, scale]() {
        return next();
        });
    measureTime("test1 sin", [&d, scale]() {
        double x = std::sin(next());
        });
    measureTime("test1 null again", [&d, scale]() {
        return next();
        });
#endif

#if 0
    measureTime("test1 add", [&d]() {
        d += .01;
        return d;
        });

    measureTime("test1 malloc", [&d]() {
        int * p = new int[16000];
        delete p;
});
#endif
}
void perfTest()
{
    test1();
}