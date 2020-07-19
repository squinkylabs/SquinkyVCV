#include "asserts.h"
#include "SubVCO.h"

#include <memory>


using oscillator = std::function<float()>;

using subvco = VoltageControlledOscillator<16, 16, rack::simd::float_4, rack::simd::int32_4>;

float vcoProfiler(bool highToLow, int minimumSamples, oscillator osc)
{
    float last = osc();
    bool done = false;
    int ct = 0;
    double sum = 0;
    int samples = 0;
    bool firstCross = true;


    while(!done) {
        float x = osc();
        bool crossed = highToLow ? 
            (x < (last - .5)) :
            (x > (last + .5));

        if (crossed) {
            if (firstCross) {
                firstCross = false;
                sum = 0;
                samples = 0;
            } else {
                done = samples > minimumSamples;
            }
        }
        if (ct > minimumSamples * 2) {
            assert(false);
            done = true;
        }
        ++ct;
        last = x;

        sum += x;
        ++samples;
    }
    return sum/samples;
}

oscillator makeFake(float freq) {
    std::shared_ptr<float> p_phase = std::make_shared<float>(0);
     oscillator osc = [p_phase]() {
        *p_phase += .01;
        if (*p_phase > 1) {
            *p_phase -= 1;
        }
        return *p_phase - .5f;
    };
    return osc;
}

oscillator makeSubSaw(float freq) {
    std::shared_ptr<subvco> osc = std::make_shared<subvco>();
    //float_4 mainIsSawMask = bitfieldToMask(0xf);
    osc->index = 0;
    float_4 mask = float_4::mask();
    osc->setWaveform(mask,mask);
    osc->setupSub(4, float_4(.01), int32_4(2), int32_4(2));

    float deltaTime = 1.f / 44100.f;

    oscillator ret = [osc, deltaTime]() {
        osc->process(deltaTime, float_4(0));
        return osc->main()[0];
    };
    return ret;
    
}

static void testDC0()
{
    auto osc = makeFake (.01);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc0 = %f\n", x);
    fflush(stdout);
    assertLE(std::abs(x), .001);
}

static void testDC1()
{
    printf("start testDC1\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.01);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc subsaw .01 = %f\n", x);
    fflush(stdout);
}

static void testDC2()
{
    printf("start testDC1\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.001);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc subsaw .001 = %f\n", x);
    fflush(stdout);
}

static void testDC3()
{
    printf("start testDC1\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.04);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc subsaw .04 = %f\n", x);
    fflush(stdout);
}
#if 0
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
#endif

void testDC()
{
    testDC0();
    testDC1();
    testDC2();
testDC3();
}