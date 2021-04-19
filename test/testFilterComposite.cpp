
#include "F2_Poly.h"
//#include "F4.h"
#include "TestComposite.h"
#include "asserts.h"
#include "simd.h"
#include "tutil.h"

using Comp2_Poly = F2_Poly<TestComposite>;

/*
 * These tests show that a) Fc follows CV, b) changing fc attenuverters forces a re-calb  
 */
static void testF2Fc_Poly(float fcParam, float cv, float expectedFcGain, float expectedFcGainWithCV, int channel) {
    // SQINFO("\n---- testF2Fc_Poly %f, %f, %f, %f", fcParam, cv, expectedFcGain, expectedFcGainWithCV);

    auto setup1 = [fcParam, cv, channel](Comp2_Poly& comp) {
        // here we will leave the attenuverters on 0
        comp.params[Comp2_Poly::FC_PARAM].value = fcParam;
        comp.inputs[Comp2_Poly::FC_INPUT].setVoltage(cv, channel);
        comp.inputs[Comp2_Poly::AUDIO_INPUT].channels = 16;

        auto xx = comp.inputs[Comp2_Poly::FC_INPUT].getVoltage(channel);
    };

    auto setup2 = [expectedFcGain, channel](Comp2_Poly& comp) {
        // this is the old expected gain
        // since trim is down, and cv has no effect, all should be as expected
        for (int i = 0; i < 16; ++i) {
            const int bank = i / 4;
            const int subChannel = i % 4;
            assertClosePct(comp._params1(bank)._fcGain()[subChannel], expectedFcGain, 10);
        }

        // now turn up the attenuverter
        comp.params[Comp2_Poly::FC_TRIM_PARAM]
            .value = 1;
    };

    auto validate = [expectedFcGainWithCV, channel, cv](Comp2_Poly& comp) {
        for (int i = 0; i < 16; ++i) {
            const int bank = i / 4;
            const int subChannel = i % 4;
            if (i == channel || cv < .01) {
                assertClosePct(comp._params1(bank)._fcGain()[subChannel], expectedFcGainWithCV, 10);
            }
            else {
                assertNotClosePct(comp._params1(bank)._fcGain()[subChannel], expectedFcGainWithCV, 10);
            }
        }
    };
    testArbitrary3<Comp2_Poly>(setup1, setup2, validate);
}

static void testF2Fc_Poly(int channel) {
    testF2Fc_Poly(0, 0, .00058f, .00058f, channel);     // no change when trim changed, becuase no CV
    testF2Fc_Poly(5, 1, 0.0186376f, 0.0372752f, channel);  // now allow some CV
    testF2Fc_Poly(5, 5, .0186f, .596f, channel);
}

static void testF2Fc_Poly() {
    for (int channel = 0; channel < 16; ++channel) {
        testF2Fc_Poly(channel);
    }
}


//--------------------------------------------------------------------------------
static void testF2Q_Poly(float qParam, float qcv, float expectedQGain, float expectedQGainWithCV, int channel) {
   // SQINFO("\n---- testF2Q_Poly ch=%d %f, %f, %f, %f", channel, qParam, qcv, expectedQGain, expectedQGainWithCV);

    auto setup1 = [qParam, qcv, channel](Comp2_Poly& comp) {
        comp.params[Comp2_Poly::Q_PARAM].value = qParam;
        comp.inputs[Comp2_Poly::Q_INPUT].setVoltage(qcv, channel);
        comp.inputs[Comp2_Poly::AUDIO_INPUT].channels = 16;
    };

    auto setup2 = [expectedQGain](Comp2_Poly& comp) {
      //  simd_assertClosePct(comp._params1(0)._qGain(), float_4(expectedQGain), 10);

        for (int i = 0; i < 16; ++i) {
            const int bank = i / 4;
            const int subChannel = i % 4;
            assertClosePct(comp._params1(bank)._qGain()[subChannel], expectedQGain, 10);
        }

        // now turn up the attenuverter
        comp.params[Comp2_Poly::Q_TRIM_PARAM].value = 1;
    };

    auto validate = [expectedQGainWithCV, channel, qcv](Comp2_Poly& comp) {
       
        for (int i = 0; i < 16; ++i) {
            const int bank = i / 4;
            const int subChannel = i % 4;
            // SQINFO("in validate, q gain = %f sub=%d i=%d", comp._params1(bank)._qGain()[subChannel], subChannel, i);
            if (i == channel || qcv < .01) {
                assertClosePct(comp._params1(bank)._qGain()[subChannel], expectedQGainWithCV, 10);
            }
            else {
                assertNotClosePct(comp._params1(bank)._qGain()[subChannel], expectedQGainWithCV, 10);
            }
        }
    };
    testArbitrary3<Comp2_Poly>(setup1, setup2, validate);
}

static void testF2Q_Poly(int channel) {
   // testF2Q_Poly(0, 0, 2, 2, channel);
  //  testF2Q_Poly(0, -10, 2, 2, channel);        // negative q gets clipped?
    testF2Q_Poly(1, 2, .916f, .285f, channel);

   // testF2Q_Poly(5, 0, .104f, .104f, channel);
  //  testF2Q_Poly(5, 10, .104f, .0098f, channel);
}

static void testF2Q_Poly() {
    for (int channel = 0; channel < 16; ++channel) {
        testF2Q_Poly(channel);
    }
}

static void testF2R_Poly(float rParam, float rcv, float fcParam, float expectedFcGain1, float expectedFcGain2) {
    auto setup = [rParam, rcv, fcParam](Comp2_Poly& comp) {
        comp.params[Comp2_Poly::R_PARAM].value = rParam;
        comp.params[Comp2_Poly::FC_PARAM].value = fcParam;
        comp.inputs[Comp2_Poly::R_INPUT].setVoltage(rcv, 0);
        comp.inputs[Comp2_Poly::FC_INPUT].setVoltage(rcv, 0);
        comp.inputs[Comp2_Poly::AUDIO_INPUT].channels = 4;
    };

    auto validate = [expectedFcGain1, expectedFcGain2](Comp2_Poly& comp) {
        simd_assertClosePct(comp._params1(0)._fcGain(), float_4(expectedFcGain1), 10);
        simd_assertClosePct(comp._params2(0)._fcGain(), float_4(expectedFcGain2), 10);
    };
    testArbitrary<Comp2_Poly>(setup, validate);
}



static void testF2R_Poly() {
    //  void testF2R(float rParam, float rcv, float fcParam, float expectedFcGain1, float expectedFcGain2)
    testF2R_Poly(0, 0, 5, .0186377f, .0186377f);
    testF2R_Poly(5, 0, 5, 0.0186376f, 0.0186376f);
    testF2R_Poly(10, 0, 2.5, 0.00329471f, .00329471f);
}

#if 0
static void testF4Fc() {
    testF2Fc<Comp4>(0, 0, .00058f);
}
#endif
static void testPolyChannelsF2() {
    testPolyChannels<Comp2_Poly>(Comp2_Poly::AUDIO_INPUT, Comp2_Poly::AUDIO_OUTPUT, 16);
}

float qFunc(float qV, int numStages) {
    assert(qV >= 0);
    assert(qV <= 10);
    assert(numStages >= 1 && numStages <= 2);

    const float expMult = (numStages == 1) ? 1 / 1.5f : 1 / 2.5f;
    float q = std::exp2(qV * expMult) - .5f;
    return q;
}

float_4 fastQFunc(float_4 qV, int numStages) {
    //  assert(qV >= 0);
    //   assert(qV <= 10);
    assert(numStages >= 1 && numStages <= 2);

    const float expMult = (numStages == 1) ? 1 / 1.5f : 1 / 2.5f;
    float_4 q = rack::dsp::approxExp2_taylor5(qV * expMult) - .5;
    return q;
}

std::pair<float, float> fcFunc(float freqVolts, float rVolts) {
    float r = std::exp2(rVolts / 3.f);
    float freq = rack::dsp::FREQ_C4 * std::exp2(freqVolts + 30 - 4) / 1073741824;

    float f1 = freq / r;
    float f2 = freq * r;
    return std::make_pair(f1, f2);
}

std::pair<float_4, float_4> fastFcFunc(float_4 freqVolts, float_4 rVolts) {
    float_4 r = rack::dsp::approxExp2_taylor5(rVolts / 3.f);
    float_4 freq = rack::dsp::FREQ_C4 * rack::dsp::approxExp2_taylor5(freqVolts + 30 - 4) / 1073741824;

    float_4 f1 = freq / r;
    float_4 f2 = freq * r;
    return std::make_pair(f1, f2);
}

static void testQFunc() {
    const int numStages = 2;
    for (float qv = 0; qv <= 10; qv += 1) {
        const float x = qFunc(qv, numStages);
        const float y = fastQFunc(qv, numStages)[0];

        float error = abs(x - y);
        float error_pct = 100 * error / y;
        assert(error_pct < 1);
    }
}

static void testFcFunc() {
    for (float fv = 0; fv <= 10; fv += 1) {
        for (float rv = 0; rv <= 10; rv += 1) {
            auto fr = fcFunc(fv, rv);
            auto ffr = fastFcFunc(fv, rv);

            float error1 = abs(fr.first - ffr.first[0]);
            float error_pct1 = 100 * error1 / fr.first;
            assert(error_pct1 < 1);

            float error2 = abs(fr.second - ffr.second[0]);
            float error_pct2 = 100 * error2 / fr.second;
            assert(error_pct2 < 1);
        }
    }
}

static void clearRandQandLimiterAndFc(Comp2_Poly& comp) {
    comp.params[Comp2_Poly::Q_PARAM].value = 0;
    comp.params[Comp2_Poly::R_PARAM].value = 0;
    comp.params[Comp2_Poly::FC_PARAM].value = 0;
    comp.params[Comp2_Poly::LIMITER_PARAM].value = 0;
}

static void testPolyFc(int chan) {
    SQINFO("---------------- test poly fc ----------");
    auto setup1 = [chan](Comp2_Poly& comp) {
        // full poly, 10V input on test
        comp.inputs[Comp2_Poly::AUDIO_INPUT].channels = 16;
        clearRandQandLimiterAndFc(comp);

        comp.inputs[Comp2_Poly::AUDIO_INPUT].setVoltage(10, chan);
    };

    auto setup2 = [chan](Comp2_Poly& comp) {
        // full poly, input on test, fc up
        comp.inputs[Comp2_Poly::AUDIO_INPUT].channels = 16;
        clearRandQandLimiterAndFc(comp);

        comp.inputs[Comp2_Poly::AUDIO_INPUT].setVoltage(10, chan);
        comp.inputs[Comp2_Poly::FC_INPUT].setVoltage(10, chan);
        comp.params[Comp2_Poly::FC_TRIM_PARAM].value = 1;
    };

    auto validate = [chan](Comp2_Poly& comp1, Comp2_Poly& comp2) {
        //  assertClosePct(comp._params1()._fcGain(), expectedFcGain, 10);
        const float v1 = comp1.outputs[Comp2_Poly::AUDIO_OUTPUT].getVoltage(chan);
        const float v2 = comp2.outputs[Comp2_Poly::AUDIO_OUTPUT].getVoltage(chan);

        SQINFO("comp outputs = %f, %f", comp1.outputs[Comp2_Poly::AUDIO_OUTPUT].getVoltage(chan), comp2.outputs[Comp2_Poly::AUDIO_OUTPUT].getVoltage(chan));
        assertGT(v2, v1 + 5);  // the one fully open should pass more
        if (chan > 0) assertEQ(comp1.outputs[Comp2_Poly::AUDIO_OUTPUT].getVoltage(chan - 1), 0);
        if (chan < 15) assertEQ(comp1.outputs[Comp2_Poly::AUDIO_OUTPUT].getVoltage(chan + 1), 0);
    };
    testArbitrary2<Comp2_Poly>(setup1, setup2, validate);
}

static void testPolyFc() {
    for (int i = 0; i < 16; ++i) {
        testPolyFc(i);
    }
}

void testFilterComposites() {
    testF2Fc_Poly();
    testF2Q_Poly();
    testF2R_Poly();
    testQFunc();
    testFcFunc();
    testPolyChannelsF2();
    testPolyFc();
}
