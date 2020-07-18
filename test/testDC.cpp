#include "asserts.h"


using oscillator = std::function<float()>;
void vcoProfiler(bool highToLow, int minimumSamples, oscillator osc)
{
    float last = osc();
    bool done = false;
    int ct = 0;
    float sum = 0;
    int samples = 0;
    bool firstCross = true;


    while(!done) {
        float x = osc();
        //printf("in loop, x = %f last = %f\n", x, last);
        bool crossed = highToLow ? 
            (x < (last - .5)) :
            (x > (last + .5));

        if (crossed) {
            printf("crossed at %d\n", ct);
            if (firstCross) {
                firstCross = false;
                sum = 0;
                samples = 0;
            } else {
                printf("finished. sum = %f norm= %f\n", sum, sum / samples);
                done = samples > minimumSamples;
            }
        }
        if (ct > 1000000) {
            assert(false);
            done = true;
        }
        ++ct;
        last = x;

        sum += x;
        ++samples;
    }
}

static void testDC0()
{
    float phase = 0;
    oscillator osc = [&phase]() {
        phase += .01;
        if (phase > 1) {
            phase -= 1;
        }
        return phase - .5f;
    };
    vcoProfiler(true, 41000, osc);
}

void testDC()
{
    testDC0();
}