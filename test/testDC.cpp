#include "asserts.h"
#include "SubVCO.h"

#include <memory>


using oscillator = std::function<float()>;

using subvco = VoltageControlledOscillator<16, 16, rack::simd::float_4, rack::simd::int32_4>;

// This version mesures full periods
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

// this simple version does not look at periods
float vcoProfiler2(int samples, oscillator osc)
{
    double sum = 0;
    int count = 0;
    while (count < samples) {
        auto x = osc();
        sum += x;
        ++count;
    }
    return sum / count;
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
    osc->setupSub(4, float_4(freq), int32_4(2), int32_4(2));

    float deltaTime = 1.f / 44100.f;

    oscillator ret = [osc, deltaTime]() {
        osc->process(deltaTime, float_4(0));
        return osc->main()[0];
    };
    return ret;
}

oscillator makeSubSq(float freq, float pw) {
    std::shared_ptr<subvco> osc = std::make_shared<subvco>();
    //float_4 mainIsSawMask = bitfieldToMask(0xf);
    osc->index = 0;
    float_4 mask = float_4::zero();
    simd_assertMask(mask);
    osc->setWaveform(mask,mask);
    osc->setupSub(4, float_4(freq), int32_4(2), int32_4(2));
    osc->setPW(float_4(pw));

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

static void testDCSaw1()
{
    printf("start testDCSaw1\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.01);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc subsaw .01 = %f\n", x);
    fflush(stdout);
}

static void testDCSaw2()
{
    printf("start testDCSaw2\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.001);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc subsaw .001 = %f\n", x);
    fflush(stdout);
}

static void testDCSaw3()
{
    printf("start testDCSaw3\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.04);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc subsaw .04 = %f\n", x);
    fflush(stdout);
}


static void testDCSaw4()
{
    printf("start testDCSaw4\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.0001);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc subsaw4 .00001 = %f\n", x);
    fflush(stdout);
}

static void testDCSaw5()
{
    printf("start testSaw5\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.0001);
    float x = vcoProfiler2(41000 * 100, osc);
    printf("final simple dc subsaw4 .00001 = %f\n", x);
    fflush(stdout);
}


static void testDCSaw6()
{
    printf("start testSaw6\n");
    fflush(stdout);    
    auto osc = makeSubSaw (.00001);
    float x = vcoProfiler2(41000 * 100, osc);
    printf("final simple dc subsaw4 .000001 = %f\n", x);
    fflush(stdout);
}

static void testDCSq1()
{
    printf("start testDCSq1\n");
    fflush(stdout);    
    auto osc = makeSubSq (.04, .5);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc sub sq .04 = %f\n", x);
    fflush(stdout);
}

static void testDCPw1()
{
    printf("start testDCPw1\n");
    fflush(stdout);    
    auto osc = makeSubSq (.04, .1);
    float x = vcoProfiler(true, 41000 * 100, osc);
    printf("final dc sub pw .04 = %f\n", x);
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
    testDCSaw1();
    testDCSaw2();
    testDCSaw3();
    testDCSaw4();
    testDCSaw5();
    testDCSaw6();
    testDCSq1();
    testDCPw1();
}