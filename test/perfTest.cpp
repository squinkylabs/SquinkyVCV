#include <functional>
#include <time.h>
#include <cmath>
#include <limits>

#include "TestComposite.h"
#include "AudioMath.h"

#include "BiquadParams.h"
#include "BiquadFilter.h"
#include "BiquadState.h"
#include "ColoredNoise.h"
#include "CompCurves.h"
#include "FrequencyShifter.h"
#include "HilbertFilterDesigner.h"
#include "LookupTableFactory.h"
#include "TestComposite.h"
#include "Tremolo.h"
#include "VocalAnimator.h"
#include "VocalFilter.h"
#include "LFN.h"
#include "LFNB.h"
#include "GMR.h"
#include "CHB.h"
#include "FunVCOComposite.h"
//#include "EV3.h"
#include "daveguide.h"
#include "Shaper.h"
#include "Super.h"
#include "KSComposite.h"
#include "Seq.h"

//#ifndef _MSC_VER
#if 1
#include "WVCO.h"
#include "Sub.h"
#include "Sines.h"
#include "Basic.h"
#endif

extern double overheadInOut;
extern double overheadOutOnly;

using Shifter = FrequencyShifter<TestComposite>;
using Animator = VocalAnimator<TestComposite>;
using VocFilter = VocalFilter<TestComposite>;
using Colors = ColoredNoise<TestComposite>;
using Trem = Tremolo<TestComposite>;


#include "MeasureTime.h"

#ifdef _USE_WINDOWS_PERFTIME
double SqTime::frequency = 0;
#endif

// There are many tests that are disabled with #if 0.
// In most cases they still work, but don't need to be run regularly

#if 0
static void test1()
{
    double d = .1;
    srand(57);
    const double scale = 1.0 / RAND_MAX;



    MeasureTime<float>::run("test1 sin", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<double>::run("test1 sin double", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<float>::run("test1 sinx2 float", []() {
        float x = std::sin(TestBuffers<float>::get());
        x = std::sin(x);
        return x;
        }, 1);

    MeasureTime<float>::run("mult float-10", []() {
        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        return x * y;
        }, 10);

    MeasureTime<double>::run("mult dbl", []() {
        double x = TestBuffers<double>::get();
        double y = TestBuffers<double>::get();
        return x * y;
        }, 1);

    MeasureTime<float>::run("div float", []() {
        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        return x / y;
        }, 1);

    MeasureTime<double>::run("div dbl", []() {
        double x = TestBuffers<double>::get();
        double y = TestBuffers<double>::get();
        return x / y;
        }, 1);

    MeasureTime<float>::run("test1 (do nothing)", [&d, scale]() {
        return TestBuffers<float>::get();
        }, 1);

    MeasureTime<float>::run("test1 pow2 float", []() {
        float x = std::pow(2, TestBuffers<float>::get());
        return x;
        }, 1);
    MeasureTime<float>::run("test1 pow rnd float", []() {
        float x = std::pow(TestBuffers<float>::get(), TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<float>::run("test1 exp float", []() {
        float x = std::exp(TestBuffers<float>::get());
        return x;
        }, 1);
}
#endif

template <typename T>
static void testHilbert()
{
    BiquadParams<T, 3> paramsSin;
    BiquadParams<T, 3> paramsCos;
    BiquadState<T, 3> state;
    HilbertFilterDesigner<T>::design(44100, paramsSin, paramsCos);

    MeasureTime<T>::run("hilbert", [&state, &paramsSin]() {

        T d = BiquadFilter<T>::run(TestBuffers<T>::get(), state, paramsSin);
        return d;
        }, 1);
}


#include "LookupTable.h"
#include <memory>
// stolen from imp in ObjectCache
std::shared_ptr<LookupTableParams<float>> makeSinTable()
{
    auto ret = std::make_shared<LookupTableParams<float>>();
    std::function<double(double)> f = AudioMath::makeFunc_Sin();
        // Used to use 4096, but 512 gives about 92db  snr, so let's save memory
    LookupTable<float>::init(*ret, 512, 0, 1, f);
    return ret;
}



// #ifndef _MSC_VER 
#if 1

static float pi =  3.141592653589793238;
inline float_4 sine2(float_4 _x)
{
    float_4 xneg = _x < float_4::zero();
    float_4 xOffset = SimdBlocks::ifelse(xneg, float_4(pi / 2.f), float_4(-pi  / 2.f));
    xOffset += _x;
    float_4 xSquared = xOffset * xOffset;
#if 0
    printf("\n*** in simdsin(%s) xsq=%s\n xoff=%s\n",
        toStr(_x).c_str(),
        toStr(xSquared).c_str(),
        toStr(xOffset).c_str());
#endif

    float_4 ret = xSquared * float_4(1.f / 24.f);
    float_4 correction = ret * xSquared *  float_4(.02 / .254);
    ret += float_4(-.5);
    ret *= xSquared;
    ret += float_4(1.f);

    ret -= correction;
    return SimdBlocks::ifelse(xneg, -ret, ret); 
}

static void simd_testSin()
{
    MeasureTime<float>::run(overheadInOut, "sin simd approx X4", []() {
        float_4 x(TestBuffers<float>::get());
        float d = rack::simd::sin(x)[0];
        return d;

        }, 1);
}

static void testSinLookupf()
{
    auto params = makeSinTable();
    MeasureTime<float>::run(overheadInOut, "sin table lookup f", [params]() {
        float d = LookupTable<float>::lookup(*params, TestBuffers<float>::get());
        return d;
        }, 1);
}

static void testSinLookupSimd()
{
    auto params = makeSinTable();
    MeasureTime<float>::run(overheadInOut, "sin approx bgf simd", [params]() {
       // float_4 d =  lookupSimd(*params, TestBuffers<float>::get(), true); //LookupTable<float>::lookup(*params, TestBuffers<float>::get());
        float_4 d = sine2(TestBuffers<float>::get());
        return d[2];
        }, 1);
}

static void testCompressorLookup()
{
    CompCurves::Recipe r;
    r.ratio = 4;
    r.kneeWidth = 0;
    auto table = CompCurves::makeCompGainLookup(r);
    MeasureTime<float>::run(overheadInOut, "non-uniform comp lookup", [table]() {
       // float_4 d =  lookupSimd(*params, TestBuffers<float>::get(), true); //LookupTable<float>::lookup(*params, TestBuffers<float>::get());
       // float_4 d = sine2(TestBuffers<float>::get());
        float d = CompCurves::lookup(table, TestBuffers<float>::get());
        return d;
        }, 1);
}
#endif



static void testSinLookup()
{
    auto params = ObjectCache<float>::getSinLookup();
    MeasureTime<float>::run(overheadInOut, "sin table lookup", [params]() {
        float d = LookupTable<float>::lookup(*params, TestBuffers<float>::get());
        return d;
        }, 1);
}


static void testShifter()
{
    Shifter fs;

    fs.setSampleRate(44100);
    fs.init();

    fs.inputs[Shifter::AUDIO_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "shifter", [&fs]() {
        fs.inputs[Shifter::AUDIO_INPUT].setVoltage(TestBuffers<float>::get(), 0);
        fs.step();
        return fs.outputs[Shifter::SIN_OUTPUT].getVoltage(0);
        }, 1);
}

static void testAnimator()
{
    Animator an;

    an.setSampleRate(44100);
    an.init();

    an.inputs[Shifter::AUDIO_INPUT].setVoltage(0, 0);;

    MeasureTime<float>::run(overheadInOut, "animator", [&an]() {
        an.inputs[Shifter::AUDIO_INPUT].setVoltage(TestBuffers<float>::get(), 0);
        an.step();
        return an.outputs[Shifter::SIN_OUTPUT].getVoltage(0);
        }, 1);
}


static void testVocalFilter()
{
    VocFilter an;

    an.setSampleRate(44100);
    an.init();

    an.inputs[Shifter::AUDIO_INPUT].setVoltage(0, 0);

    MeasureTime<float>::run(overheadInOut, "vocal filter", [&an]() {
        an.inputs[Shifter::AUDIO_INPUT].setVoltage(TestBuffers<float>::get(), 0);
        an.step();
        return an.outputs[Shifter::SIN_OUTPUT].getVoltage(0);
        }, 1);
}

static void testColors()
{
    Colors co;

    co.setSampleRate(44100);
    co.init();


    MeasureTime<float>::run(overheadInOut, "colors", [&co]() {
        co.step();
        return co.outputs[Colors::AUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testTremolo()
{
    Trem tr;

    tr.setSampleRate(44100);
    tr.init();


    MeasureTime<float>::run(overheadInOut, "trem", [&tr]() {
        tr.inputs[Trem::AUDIO_INPUT].setVoltage(TestBuffers<float>::get(), 0);
        tr.step();
        return tr.outputs[Trem::AUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testLFN()
{
    LFN<TestComposite> lfn;

    lfn.setSampleTime(1.0f / 44100.f);
    lfn.init();

    MeasureTime<float>::run(overheadOutOnly, "lfn", [&lfn]() {
        lfn.step();
        return lfn.outputs[LFN<TestComposite>::OUTPUT].getVoltage(0);
        }, 1);
}

static void testLFNB()
{
    LFNB<TestComposite> lfn;

 

 //   lfn.setSampleTime(1.0f / 44100.f);
    lfn.onSampleRateChange();
    lfn.init();
    

    MeasureTime<float>::run(overheadOutOnly, "lfnb", [&lfn]() {
        lfn.step();
        return lfn.outputs[LFNB<TestComposite>::AUDIO0_OUTPUT].getVoltage(0);
        }, 1);
}
#if 0
static void testEvenOrig()
{
    EvenVCO_orig<TestComposite> lfn;

    lfn.outputs[EvenVCO_orig<TestComposite>::EVEN_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO_orig<TestComposite>::SINE_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO_orig<TestComposite>::TRI_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO_orig<TestComposite>::SQUARE_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO_orig<TestComposite>::SAW_OUTPUT].channels = 1;

    for (int i = 0; i < 100; ++i) lfn.step();

    MeasureTime<float>::run(overheadOutOnly, "Even orig", [&lfn]() {
        lfn.inputs[EvenVCO_orig<TestComposite>::PITCH1_INPUT].value = TestBuffers<float>::get();
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].value;
        }, 1);
}
#endif

#if 0
static void testEven()
{
    EvenVCO<TestComposite> lfn;


    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].channels = 1;
    MeasureTime<float>::run(overheadOutOnly, "Even, all outs", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].value;
        }, 1);
}

static void testEvenEven()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].channels = 0;

    MeasureTime<float>::run(overheadOutOnly, "Even, even only", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].value;
        }, 1);
}

static void testEvenSin()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].channels = 0;

    MeasureTime<float>::run(overheadOutOnly, "Even, sin only", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].value;
        }, 1);
}

static void testEvenSaw()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].channels = 1;

    for (int i = 0; i < 100; ++i) lfn.step();

    MeasureTime<float>::run(overheadOutOnly, "Even, saw only", [&lfn]() {
        lfn.inputs[EvenVCO<TestComposite>::PITCH1_INPUT].value = TestBuffers<float>::get();
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].value;
        }, 1);
}


static void testEvenTri()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].channels = 0;

    MeasureTime<float>::run(overheadOutOnly, "Even, tri only", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].value;
        }, 1);
}

static void testEvenSq()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].channels = 0;

    MeasureTime<float>::run(overheadOutOnly, "Even, Sq only", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].value;
        }, 1);
}

static void testEvenSqSaw()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].channels = 0;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].channels = 1;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].channels = 1;

    MeasureTime<float>::run(overheadOutOnly, "Even, Sq Saw", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].value;
        }, 1);
}
#endif

static void testFun()
{
    FunVCOComposite<TestComposite> lfn;

    for (int i = 0; i < lfn.NUM_OUTPUTS; ++i) {
        lfn.outputs[i].channels = 1;
    }

    lfn.setSampleRate(44100.f);
    const bool isAnalog = false;
    lfn.params[FunVCOComposite<TestComposite>::MODE_PARAM].value = isAnalog ? 1.0f : 0.f;

    MeasureTime<float>::run(overheadOutOnly, "Fun all on, digital", [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].getVoltage(0) +
            lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].getVoltage(0) +
            lfn.outputs[FunVCOComposite<TestComposite>::SIN_OUTPUT].getVoltage(0) +
            lfn.outputs[FunVCOComposite<TestComposite>::SQR_OUTPUT].getVoltage(0);
        }, 1);
}

static void testFunNone()
{
    FunVCOComposite<TestComposite> lfn;

    for (int i = 0; i < lfn.NUM_OUTPUTS; ++i) {
        lfn.outputs[i].channels = 0;
    }

    lfn.setSampleRate(44100.f);

    MeasureTime<float>::run(overheadOutOnly, "Fun all off", [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].getVoltage(0);
        }, 1);
}

static void testFunSaw(bool isAnalog)
{
    FunVCOComposite<TestComposite> lfn;

    lfn.outputs[FunVCOComposite<TestComposite>::SIN_OUTPUT].channels = 0;
    lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].channels = 0;
    lfn.outputs[FunVCOComposite<TestComposite>::SQR_OUTPUT].channels = 0;
    lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].channels = 1;

    //  oscillator.analog = TBase::params[MODE_PARAM].value > 0.0f;
    lfn.params[FunVCOComposite<TestComposite>::MODE_PARAM].value = isAnalog ? 1.0f : 0.f;

    lfn.setSampleRate(44100.f);

    std::string title = isAnalog ? "Fun Saw Analog" : "Fun Saw Digital";
    MeasureTime<float>::run(overheadOutOnly, title.c_str(), [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].getVoltage(0);
        }, 1);
}

static void testFunSin(bool isAnalog)
{
    FunVCOComposite<TestComposite> lfn;

    lfn.outputs[FunVCOComposite<TestComposite>::SIN_OUTPUT].channels = 1;
    lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].channels = 0;
    lfn.outputs[FunVCOComposite<TestComposite>::SQR_OUTPUT].channels = 0;
    lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].channels = 0;

    lfn.params[FunVCOComposite<TestComposite>::MODE_PARAM].value = isAnalog ? 1.0f : 0.f;

    lfn.setSampleRate(44100.f);

    std::string title = isAnalog ? "Fun Sin Analog" : "Fun Sin Digital";
    MeasureTime<float>::run(overheadOutOnly, title.c_str(), [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].getVoltage(0);
        }, 1);
}

static void testFunSq()
{
    FunVCOComposite<TestComposite> lfn;

    lfn.outputs[FunVCOComposite<TestComposite>::SIN_OUTPUT].channels = 0;
    lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].channels = 0;
    lfn.outputs[FunVCOComposite<TestComposite>::SQR_OUTPUT].channels = 1;
    lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].channels = 0;

    lfn.setSampleRate(44100.f);

    MeasureTime<float>::run(overheadOutOnly, "Fun sq", [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].getVoltage(0);
        }, 1);
}

static void testCHBdef()
{
    CHB<TestComposite> chb;
    std::string name = "chbdef ";
    MeasureTime<float>::run(overheadOutOnly, name.c_str(), [&chb]() {
        chb.step();
        return chb.outputs[CHB<TestComposite>::MIX_OUTPUT].getVoltage(0);
        }, 1);
}

#if 0
static void testEV3()
{
    EV3<TestComposite> ev3;

    MeasureTime<float>::run(overheadOutOnly, "ev3", [&ev3]() {
        ev3.step();
        return ev3.outputs[EV3<TestComposite>::MIX_OUTPUT].value;
        }, 1);
}
#endif

static void testGMR()
{
    GMR<TestComposite> gmr;

    gmr.setSampleRate(44100);
    gmr.init();

    MeasureTime<float>::run(overheadOutOnly, "gmr", [&gmr]() {
        gmr.step();
        return gmr.outputs[GMR<TestComposite>::TRIGGER_OUTPUT].getVoltage(0);
        }, 1);
}

static void testDG()
{
    Daveguide<TestComposite> gmr;

   // gmr.setSampleRate(44100);
   // gmr.init();

    MeasureTime<float>::run(overheadOutOnly, "dg", [&gmr]() {
        gmr.step();
        return gmr.outputs[GMR<TestComposite>::TRIGGER_OUTPUT].getVoltage(0);
        }, 1);
}

// 95
// down to 67 for just the oversampler.
static void testShaper1a()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_OVERSAMPLE].value = 0;
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::FullWave;

    MeasureTime<float>::run(overheadOutOnly, "shaper fw 16X", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO0].setVoltage(TestBuffers<float>::get(), 0);
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO0].getVoltage(0);
        }, 1);
}

static void testShaper1b()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::FullWave;
    gmr.params[Shaper<TestComposite>::PARAM_OVERSAMPLE].value = 1;

    MeasureTime<float>::run(overheadOutOnly, "shaper fw 4X", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO0].setVoltage(TestBuffers<float>::get(), 0);
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO0].getVoltage(0);
        }, 1);
}


static void testBiquad()
{
    BiquadParams<float, 3> params;
    BiquadState<float, 3> state;
    ButterworthFilterDesigner<float>::designSixPoleLowpass(params, .1f);
   
    MeasureTime<float>::run(overheadInOut, "6p LP", [&state, &params]() {
        float d = BiquadFilter<float>::run(TestBuffers<float>::get(), state, params);
        return d;
        }, 1);
}

//#ifndef _MSC_VER
#if 1
 
static void simd_testBiquad()
{
    BiquadParams<float_4, 3> params;
    BiquadState<float_4, 3> state;
    ButterworthFilterDesigner<float_4>::designSixPoleLowpass(params, .1);
   
    MeasureTime<float>::run(overheadInOut, "6p LP x4 simd", [&state, &params]() {
        float_4 d = BiquadFilter<float_4>::run(TestBuffers<float>::get(), state, params);
        return d[0];
        }, 1);
}

static void testWVCOPoly()
{
    printf("starting poly svco\n"); fflush(stdout);
    WVCO<TestComposite> wvco;

    wvco.init();
    wvco.inputs[WVCO<TestComposite>::MAIN_OUTPUT].channels = 8;
    wvco.inputs[WVCO<TestComposite>::VOCT_INPUT].channels = 8;
    wvco.params[WVCO<TestComposite>::WAVE_SHAPE_PARAM].value  = 0;
    MeasureTime<float>::run(overheadOutOnly, "wvco poly 8", [&wvco]() {
        wvco.step();
        return wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(0) + 
            wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(1) + 
            wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(2) + 
            wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(3) + 
            wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(4) + 
            wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(5) + 
            wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(6) + 
            wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(7);
    }, 1);
}

static void testBasic(const std::string& name, Basic<TestComposite>::Waves waveform, bool dynamicCV)
{
    printf("starting %s\n", name.c_str()); fflush(stdout);
    Basic<TestComposite> vco;

    vco.init();
    vco.inputs[Basic<TestComposite>::MAIN_OUTPUT].channels = 1;
    vco.inputs[Basic<TestComposite>::VOCT_INPUT].channels = 1;
    vco.params[Basic<TestComposite>::WAVEFORM_PARAM].value = float(waveform);


    Basic<TestComposite>::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;
    bool toggle = true;

    MeasureTime<float>::run(overheadOutOnly, name.c_str(), [&vco, args, &toggle, dynamicCV]() {
        vco.process(args);
        if (dynamicCV) {
            toggle = !toggle;
            float v = toggle ? 0 : 2.23;
            vco.inputs[Basic<TestComposite>::VOCT_INPUT].setVoltage(v, 0);
        }
        return vco.outputs[Basic<TestComposite>::MAIN_OUTPUT].getVoltage(0);           
    }, 1);
    printf("\n");  fflush(stdout);

}

static void testBasic1Tri()
{
   testBasic("basic tri 1", Basic<TestComposite>::Waves::TRI, false);
}
static void testBasic1TriDyn()
{
   testBasic("basic tri 1 dyn", Basic<TestComposite>::Waves::TRI, true);
}
static void testBasic1Sq()
{
   testBasic("basic sq 1", Basic<TestComposite>::Waves::SQUARE, false);
}
static void testBasic1SqDyn()
{
   testBasic("basic sq 1 dyn", Basic<TestComposite>::Waves::SQUARE, true);
}

static void testBasic1Sin()
{
    printf("starting basic sin 1\n"); fflush(stdout);
    Basic<TestComposite> vco;

    vco.init();
    vco.inputs[Basic<TestComposite>::MAIN_OUTPUT].channels = 1;
    vco.inputs[Basic<TestComposite>::VOCT_INPUT].channels = 1;
    vco.params[Basic<TestComposite>::WAVEFORM_PARAM].value = float(Basic<TestComposite>::Waves::SIN);


    Basic<TestComposite>::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadOutOnly, "basic 1 sin", [&vco, args]() {
        vco.process(args);
        return vco.outputs[Basic<TestComposite>::MAIN_OUTPUT].getVoltage(0);           
    }, 1);
}

static void testBasic1Saw()
{
    printf("starting basic saw 1\n"); fflush(stdout);
    Basic<TestComposite> vco;

    vco.init();
    vco.inputs[Basic<TestComposite>::MAIN_OUTPUT].channels = 1;
    vco.inputs[Basic<TestComposite>::VOCT_INPUT].channels = 1;
    vco.params[Basic<TestComposite>::WAVEFORM_PARAM].value = float(Basic<TestComposite>::Waves::SAW);


    Basic<TestComposite>::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadOutOnly, "basic 1 saw", [&vco, args]() {
        vco.process(args);
        return vco.outputs[Basic<TestComposite>::MAIN_OUTPUT].getVoltage(0);           
    }, 1);
}

static void testOrgan1()
{
    printf("starting organ 1\n"); fflush(stdout);
    Sines<TestComposite> sines;

    sines.init();
    sines.inputs[Sines<TestComposite>::MAIN_OUTPUT].channels = 1;
    sines.inputs[Sines<TestComposite>::VOCT_INPUT].channels = 1;
    sines.inputs[Sines<TestComposite>::GATE_INPUT].channels = 1;

    Sines<TestComposite>::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadOutOnly, "organ 1", [&sines, args]() {
        sines.process(args);
        return sines.outputs[Sines<TestComposite>::MAIN_OUTPUT].getVoltage(0);           
    }, 1);
}

static void testOrgan4()
{
    printf("starting organ 4\n"); fflush(stdout);
    Sines<TestComposite> sines;

    sines.init();
    sines.inputs[Sines<TestComposite>::MAIN_OUTPUT].channels = 1;
    sines.inputs[Sines<TestComposite>::VOCT_INPUT].channels = 4;
    sines.inputs[Sines<TestComposite>::GATE_INPUT].channels = 4;

     Sines<TestComposite>::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadOutOnly, "organ 4", [&sines, args]() {
        sines.process(args);
        return sines.outputs[Sines<TestComposite>::MAIN_OUTPUT].getVoltage(0);           
    }, 1);
}

static void testOrgan4VCO()
{
    printf("starting organ 4 VCO\n"); fflush(stdout);
    Sines<TestComposite> sines;

    sines.init();
    sines.inputs[Sines<TestComposite>::MAIN_OUTPUT].channels = 1;
    sines.inputs[Sines<TestComposite>::VOCT_INPUT].channels = 4;
   //sines.inputs[Sines<TestComposite>::GATE_INPUT].channels = 4;

     Sines<TestComposite>::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadOutOnly, "organ 4VCO", [&sines, args]() {
        sines.process(args);
        return sines.outputs[Sines<TestComposite>::MAIN_OUTPUT].getVoltage(0);           
    }, 1);
}

static void testOrgan12()
{
    printf("starting organ 12\n"); fflush(stdout);
    Sines<TestComposite> sines;

    sines.init();
    sines.inputs[Sines<TestComposite>::MAIN_OUTPUT].channels = 1;
    sines.inputs[Sines<TestComposite>::VOCT_INPUT].channels = 12;
    sines.inputs[Sines<TestComposite>::GATE_INPUT].channels = 12;

     Sines<TestComposite>::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadOutOnly, "organ 12", [&sines, args]() {
        sines.process(args);
        return sines.outputs[Sines<TestComposite>::MAIN_OUTPUT].getVoltage(0);           
    }, 1);
}

static void testSubMono()
{
    Sub<TestComposite> sub;

    sub.init();
    sub.inputs[Sub<TestComposite>::MAIN_OUTPUT].channels = 1;
    sub.inputs[Sub<TestComposite>::VOCT_INPUT].channels = 1;

    MeasureTime<float>::run(overheadOutOnly, "Sub mono", [&sub]() {
        sub.step();
        return sub.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(0);
    }, 1);
}

static void testSubPoly()
{
    Sub<TestComposite> sub;

    sub.init();
    sub.inputs[Sub<TestComposite>::MAIN_OUTPUT].channels = 8;
    sub.inputs[Sub<TestComposite>::VOCT_INPUT].channels = 8;

    MeasureTime<float>::run(overheadOutOnly, "Sub poly 8", [&sub]() {
        sub.step();
        return sub.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(0);
    }, 1);
}
#endif

static void testSuper()
{
    Super<TestComposite> super;

    MeasureTime<float>::run(overheadOutOnly, "super", [&super]() {
        super.step();
        return super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(0);
    }, 1);
}


static void testSuperPoly()
{
    Super<TestComposite> super;

    super.inputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].channels = 8;
    super.inputs[Super<TestComposite>::CV_INPUT].channels = 8;
    MeasureTime<float>::run(overheadOutOnly, "super poly 8 1X", [&super]() {
        super.step();
        return super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(0) + 
            super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(1) + 
            super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(2) + 
            super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(3) + 
            super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(4) + 
            super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(5) + 
            super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(6) + 
            super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(7);
    }, 1);
}

static void testSuperStereo()
{
    Super<TestComposite> super;

    super.inputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].channels = 1;
    super.inputs[Super<TestComposite>::MAIN_OUTPUT_RIGHT].channels = 1;
    MeasureTime<float>::run(overheadOutOnly, "super stereo", [&super]() {
        super.step();
        return super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(0) +
        super.outputs[Super<TestComposite>::MAIN_OUTPUT_RIGHT].getVoltage(1); 
    }, 1);
}

static void testSuper2()
{
    Super<TestComposite> super;

    super.params[Super<TestComposite>::CLEAN_PARAM].value = 1;
    MeasureTime<float>::run(overheadOutOnly, "super clean", [&super]() {
        super.step();
        return super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(0);
    }, 1);
}

static void testSuper2Stereo()
{
    Super<TestComposite> super;

    super.params[Super<TestComposite>::CLEAN_PARAM].value = 1;
    super.inputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].channels = 1;
    super.inputs[Super<TestComposite>::MAIN_OUTPUT_RIGHT].channels = 1;
    MeasureTime<float>::run(overheadOutOnly, "super stereo", [&super]() {
        super.step();
        return super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(0) +
        super.outputs[Super<TestComposite>::MAIN_OUTPUT_RIGHT].getVoltage(0); 
    }, 1);
}

static void testSuper3()
{
    Super<TestComposite> super;

    super.params[Super<TestComposite>::CLEAN_PARAM].value = 2;
    MeasureTime<float>::run(overheadOutOnly, "super clean 2", [&super]() {
        super.step();
        return super.outputs[Super<TestComposite>::MAIN_OUTPUT_LEFT].getVoltage(0);
        }, 1);
}
#if 0
static void testSuper2()
{
    Super<TestComposite> super;
    int counter = 1;
    bool flip = false;
    float cv = 0;

    MeasureTime<float>::run(overheadOutOnly, "super pitch change", [&]() {
        if (--counter == 0) {
            cv = flip ? 1.f : -1.f;
            super.inputs[Super<TestComposite>::CV_INPUT].value = cv;
            counter = 64;
            flip = !flip;
        }
        super.step();
        return super.outputs[Super<TestComposite>::MAIN_OUTPUT].value;
        }, 1);
}
#endif

static void testKS()
{
    KSComposite<TestComposite> gmr;

    MeasureTime<float>::run(overheadOutOnly, "ks", [&gmr]() {
       // gmr.inputs[KSComposite<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[KSComposite<TestComposite>::SQR_OUTPUT].getVoltage(0);
        }, 1);
}

static void testShaper1c()
{
    Shaper<TestComposite> gmr;

    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::FullWave;
    gmr.params[Shaper<TestComposite>::PARAM_OVERSAMPLE].value = 2;

    MeasureTime<float>::run(overheadOutOnly, "shaper fw 1X", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO0].setVoltage(TestBuffers<float>::get(), 0);
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO0].getVoltage(0);
        }, 1);
}

// 284
static void testShaper2()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::Crush;

    MeasureTime<float>::run(overheadOutOnly, "shaper crush", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO0].setVoltage(TestBuffers<float>::get(), 0);
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO0].getVoltage(0);
        }, 1);
}

// 143
static void testShaper3()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::AsymSpline;

    MeasureTime<float>::run(overheadOutOnly, "shaper asy", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO0].setVoltage(TestBuffers<float>::get(), 0);
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO0].getVoltage(0);
        }, 1);
}

static void testShaper4()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::Fold;

    MeasureTime<float>::run(overheadOutOnly, "folder", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO0].setVoltage(TestBuffers<float>::get(), 0);
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO0].getVoltage(0);
        }, 1);
}

static void testShaper5()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::Fold2;

    MeasureTime<float>::run(overheadOutOnly, "folder II", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO0].setVoltage(TestBuffers<float>::get(), 0);
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO0].getVoltage(0);
        }, 1);
}
#if 0
static void testAttenuverters()
{
    auto scaler = AudioMath::makeLinearScaler<float>(-2, 2);
    MeasureTime<float>::run("linear scaler", [&scaler]() {
        float cv = TestBuffers<float>::get();
        float knob = TestBuffers<float>::get();
        float trim = TestBuffers<float>::get();
        return scaler(cv, knob, trim);
        }, 1);

    LookupTableParams<float> lookup;
    LookupTableFactory<float>::makeBipolarAudioTaper(lookup);
    MeasureTime<float>::run("bipolar lookup", [&lookup]() {
        float x = TestBuffers<float>::get();
        return LookupTable<float>::lookup(lookup, x);
        }, 1);


   // auto refFuncPos = AudioMath::makeFunc_AudioTaper(LookupTableFactory<T>::audioTaperKnee());

    {

        auto bipolarScaler = [&lookup, &scaler](float cv, float knob, float trim) {
            float scaledTrim = LookupTable<float>::lookup(lookup, cv);
            return scaler(cv, knob, scaledTrim);
        };

        MeasureTime<float>::run("bipolar scaler", [&bipolarScaler]() {
            float cv = TestBuffers<float>::get();
            float knob = TestBuffers<float>::get();
            float trim = TestBuffers<float>::get();
            return bipolarScaler(cv, knob, trim);
            }, 1);
    }
}
#endif

#if 0
static void testNoise(bool useDefault)
{

    std::default_random_engine defaultGenerator{99};
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> distribution{0, 1.0};

    std::string title = useDefault ? "default random" : "fancy random";

    MeasureTime<float>::run(overheadOutOnly, title.c_str(), [useDefault, &distribution, &defaultGenerator, &gen]() {
        if (useDefault) return distribution(defaultGenerator);
        else return distribution(gen);
        }, 1);
}


static uint64_t xoroshiro128plus_state[2] = {};

static uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static uint64_t xoroshiro128plus_next(void)
{
    const uint64_t s0 = xoroshiro128plus_state[0];
    uint64_t s1 = xoroshiro128plus_state[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    xoroshiro128plus_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
    xoroshiro128plus_state[1] = rotl(s1, 36); // c

    return result;
}

float randomUniformX()
{
    // 24 bits of granularity is the best that can be done with floats while ensuring that the return value lies in [0.0, 1.0).
    return (xoroshiro128plus_next() >> (64 - 24)) / powf(2, 24);
}

float randomNormalX()
{
    // Box-Muller transform
    float radius = sqrtf(-2.f * logf(1.f - randomUniformX()));
    float theta = 2.f * M_PI * randomUniformX();
    return radius * sinf(theta);

    // // Central Limit Theorem
    // const int n = 8;
    // float sum = 0.0;
    // for (int i = 0; i < n; i++) {
    // 	sum += randomUniform();
    // }
    // return (sum - n / 2.f) / sqrtf(n / 12.f);
}

static void testNormal()
{
    MeasureTime<float>::run(overheadOutOnly, "normal", []() {
        return randomNormalX();
        }, 1);
}
#endif

void dummy()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    Seq<TestComposite> s(ms);
}

void perfTest()
{
    printf("starting perf test\n");
    fflush(stdout);
  //  setup();
    assert(overheadInOut > 0);
    assert(overheadOutOnly > 0);

     testVocalFilter();
#if 0
    testColors();
   
    testAnimator();
    testTremolo();
  
    testShifter();
    testGMR();
#endif
#ifndef _MSC_VER
    testBasic1Tri();
    testBasic1TriDyn();
    testBasic1Sin();
    testBasic1Saw();
    testBasic1Sq();
    testBasic1SqDyn();
    testOrgan1();
    testOrgan4();
    testOrgan4VCO();
    testOrgan12();
    testSuper();
    testSuperPoly();
    testWVCOPoly();
    testSubMono();
    testSubPoly();
    simd_testBiquad();
    testSinLookup();
    testSinLookupf();
    testSinLookupSimd();
    simd_testSin();
    testCompressorLookup();
#endif  

    testBiquad();

  
    testSuperStereo();
    testSuper2();
    testSuper2Stereo();
    testSuper3();
  //  testKS();
  //  testShaper1a();
    testLFN();
    testLFNB();


    testCHBdef();
#if 0
    testShaper1b();
    testShaper1c();
    testShaper2();
    testShaper3();
    testShaper4();
    testShaper5();
#endif

   // testEV3();

    testFunSaw(true);
#if 0
    testFunSaw(false);
    testFunSin(true);
    testFunSin(false);
    testFunSq();
    testFun();
    testFunNone();
#endif


#if 0
    testEven();
    testEvenEven();
    testEvenSin();
    testEvenSaw();
    testEvenTri();
    testEvenSq();
    testEvenSqSaw();
#endif




   // test1();
#if 0
    testHilbert<float>();
    testHilbert<double>();
#endif
}