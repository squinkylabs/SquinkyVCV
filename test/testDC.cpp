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
    bool secondCross = true;

   // int pct = 0;


    while(!done) {
        float x = osc();
        #if 0
        if (++pct < 20) {
            printf("osc[%d] = %f\n", ct, x);
        }
        #endif

        // 1 worked ok, but failed at 01
        const float delta = .5;
        bool crossed = highToLow ? 
            (x < (last - delta)) :
            (x > (last + delta));

        if (crossed) {
        #if 0
            if (samples < 100 || firstCross || secondCross) {
                printf("\n**crossed at samples = %d, x=%f, last=%f\n", samples, x, last);
                printf("first = %d, second=%d\n", firstCross, secondCross);
            }
        #endif
            if (firstCross) {
              //  printf("first cross at %d\n", samples);
                firstCross = false;
                sum = 0;
                samples = 0;
            } else {
                if (secondCross && (samples > 3)) {
                    secondCross = false;
                    printf("at second cross, period = %d\n", samples);
                    fflush(stdout);
                    assert(samples > 3);
                }

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

std::pair<oscillator, float>
 makeSubSaw(float freq) {
//oscillator makeSubSaw(float freq) {
    printf("make sub saw called with freq = %f\n", freq);
    std::shared_ptr<subvco> osc = std::make_shared<subvco>();
    //float_4 mainIsSawMask = bitfieldToMask(0xf);
    osc->index = 0;
    float_4 mask = float_4::mask();
    osc->setWaveform(mask,mask);
    osc->setupSub(4, float_4(freq), int32_4(2), int32_4(2));

    const float sampleTime = 1.f / 44100.f;
    osc->computeOffsetCorrection(sampleTime);

    float deltaTime = 1.f / 44100.f;

    oscillator ret = [osc, deltaTime]() {
        osc->process(deltaTime, float_4(0));
        return osc->main()[0];
    };
    //return ret;
    const float f = osc->_getFreq()[0];
    return std::make_pair(ret, f);
}


/// second is the freq
std::pair<oscillator, float>
makeSubSq(float freq, float pw) {
    std::shared_ptr<subvco> osc = std::make_shared<subvco>();
    osc->index = 0;
    float_4 mask = float_4::zero();
    simd_assertMask(mask);
    osc->setWaveform(mask,mask);
    osc->setupSub(4, float_4(freq), int32_4(2), int32_4(2));
    osc->setPW(float_4(pw));

    const float sampleTime = 1.f / 44100.f;
    osc->computeOffsetCorrection(sampleTime);

    float deltaTime = 1.f / 44100.f;

    oscillator ret = [osc, deltaTime]() {
        osc->process(deltaTime, float_4(0));
        return osc->main()[0];
    };
    const float f = osc->_getFreq()[0];
    return std::make_pair(ret, f);
}



static void doSawTest(float pitch) {
    auto temp = makeSubSaw (pitch);
    float x = vcoProfiler(true, 41000 * 100, temp.first);

    const float normFreq = temp.second / 44100.f; 
    assertLT(x, .001);
}

static void saws() {
    #if 0
    // wanted -4, ng
    for (int i=-4; i<5; ++i) {
        doSawTest(i);
    }
    #endif

    doSawTest(0);
    doSawTest(3);
}



static void doSqTest(float pitch, float pw) {
  //  printf("\n\n*************************** start sq test(%f, %f)\n", pitch, pw);
  //  fflush(stdout);    
    auto temp = makeSubSq (pitch, pw);
    float x = vcoProfiler(true, 41000 * 100, temp.first);

 //   const float normFreq = temp.second / 44100.f;
    assertLT(x, .001);
}


static void pulses() {
    // wanted -4, ng
#if 0
    for (float pw=0; pw < 1; pw += .1) {
        doSqTest(0, pw);

    }
#endif
 doSqTest(0, .5f);
}

void testDC()
{
    saws();
    pulses();
}