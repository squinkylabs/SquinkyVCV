#include "asserts.h"


using oscillator = std::function<float()>;
void vcoProfiler(bool highToLow, oscillator osc)
{
    float last = osc();
    bool done = false;
    int ct = 0;
    while(!done) {
        float x = osc();
        printf("in loop, x = %f last = %f\n", x, last);
        bool crossed = highToLow ? 
            (x < (last - .5)) :
            (x > (last + .5));

        if (crossed) {
            printf("crossed at %d\n", ct);
            done = true;
        }
        if (ct > 1000000) {
            assert(false);
            done = true;
        }
        ++ct;
        last = x;
    }
}

static void testDC0()
{
    float phase = 0;
    oscillator osc = [&phase]() {
        phase += .1;
        if (phase > 1) {
            phase -= 1;
        }
        return phase;
    };
    vcoProfiler(true, osc);
}

void testDC()
{
    testDC0();
}