#pragma once

#include "AsymWaveShaper.h"
#include "IIRDecimator.h"
#include "IIRUpsampler.h"
#include "LookupTable.h"
#include "ObjectCache.h"
#include "TrapezoidalLowpass.h"

#include <vector>
#include <string>

template <typename T>
class LadderFilter
{
public:
    LadderFilter();
    enum class Types
    {
        _4PLP,
        _3PLP,
        _2PLP,
        _1PLP,
        _2PBP,
        _2HP1LP,
        _3HP1LP,
        _4PBP,
        _1LPNotch,
        _3AP1LP,
        _3PHP,
        _2PHP,
        _1PHP,
        _NOTCH,
        _PHASER,
        NUM_TYPES
    };

    enum class Voicing
    {
        Classic,
        Clip2,
        Fold,
        Fold2,
        Clean,
        NUM_VOICINGS
    };

    void run(T);
    T getOutput();

    /**
     * input range >0 to < .5
     */
    void setNormalizedFc(T);

    void setFeedback(T f);
    void setType(Types);
    void setVoicing(Voicing);
    void setGain(T);
    void setEdge(T);        // 0..1
    void setFreqSpread(T);
    void setBassMakeupGain(T);
    void setSlope(T);       // 0..3. only works in 4 pole
    void setVolume(T vol);  // 0..1

    float getLEDValue(int tapNumber);

    static std::vector<std::string> getTypeNames();
    static std::vector<std::string> getVoicingNames();

private:
    TrapezoidalLowpass<T> lpfs[4];

    /**
     * Lowpass pole gain (base freq of filter converted)
     */
    T _g = .001f;

    /**
     * Individual _g values for each stage of filter.
     * normally all the same, but "caps" control separates them
     */
    T stageG[4] = {.001f, .001f, .001f, .001f};

   /**
    * Output mixer gain for each stage
    */
    T stageTaps[4] = {0, 0, 0, 1};

    T bassMakeupGain = 1;
    T mixedOutput = 0;
    T requestedFeedback = 0;
    T adjustedFeedback = 0;
    T gain = T(.3);
    T stageOutputs[4];
    T rawEdge = 0;
    T processedEdge = 0;
    T freqSpread = 0;
    T slope = 3;
    T volume = 0;

    T stageGain[4] = {1, 1, 1, 1};
    T stageFreqOffsets[4] = {1, 1, 1, 1};

    Types type = Types::_4PLP;
    Voicing voicing = Voicing::Classic;
    T lastNormalizedFc = T(.0001);
    T lastSlope = -1;
    T lastVolume = -1;
    T finalVolume = 0;

    bool bypassFirstStage = false;

    std::shared_ptr<NonUniformLookupTableParams<T>> fs2gLookup = makeTrapFilter_Lookup<T>();
    std::shared_ptr<LookupTableParams<T>> tanhLookup = ObjectCache<T>::getTanh5();
    std::shared_ptr<LookupTableParams<T>> expLookup = ObjectCache<T>::getExp2();

    static const int oversampleRate = 4;
    IIRUpsampler up;
    IIRDecimator down;

    AsymWaveShaper shaper;

    void runBufferClassic(float* buffer, int);
    void runBufferClip2(float* buffer, int);
    void runBufferFold(float* buffer, int);
    void runBufferFold2(float* buffer, int);
    void runBufferClean(float* buffer, int);

    void updateFilter();
    void updateSlope();
    void updateFeedback();
    void updateStageGains();
    void dump(const char* p);
    T getGfromNormFreq(T nf) const;
};

template <typename T>
LadderFilter<T>::LadderFilter()
{
    // fix at 4X oversample
    up.setup(oversampleRate);
    down.setup(oversampleRate);
}

template <typename T>
inline void LadderFilter<T>::dump(const char* p)
{
#if 0 
    printf("\ndump %s\n", p);
    printf("feedback=%.2f, gain=%.2f edge=%.2f slope=%.2f\n", feedback, gain, edge, slope);
    printf("filt:_g=%f,  bgain=%.2f bypassFirst=%d\n", _g, bassMakeupGain, bypassFirstStage);
    for (int i = 0; i < 4; ++i) {
        printf("stage[%d] tap=%.2f, gain=%.2f freqoff=%.2f filter_G %f\n", i,
            stageTaps[i],
            stageGain[i],
            stageFreqOffsets[i],
            stageG[i]);
    }
    fflush(stdout);
#endif
}

template <typename T>
inline float LadderFilter<T>::getLEDValue(int tapNumber)
{
    T ret =  (type == Types::_4PLP) ? stageTaps[tapNumber] : 0;  
    return float(ret);
}

template <typename T>
inline void  LadderFilter<T>::updateSlope()
{
    if (type != Types::_4PLP) {
        return;
    }
   
  //  printf("\n update slope %f\n", slope);
    int iSlope = (int) std::floor(slope);
    for (int i = 0; i < 4; ++i) {
        if (i == iSlope) {
            stageTaps[i] = ((i + 1) - slope) * 1;
            if (i < 3) {
                stageTaps[i + 1] = (slope - i) * 1;
            }
        } else if (i != (iSlope + 1)) {
            stageTaps[i] = 0;
        }
    }
}

template <typename T>
inline void LadderFilter<T>::setSlope(T _slope)
{
    if (slope == lastSlope) {
        return;
    }
    slope = _slope;
    slope = std::min(slope, T(3));
    slope = std::max(slope, T(0));
    updateSlope();
}

template <typename T>
inline void LadderFilter<T>::setVolume(T vol)
{
    if (lastVolume == vol) {
        return;
    }
    lastVolume = vol;
    finalVolume = 4 * vol * vol;
}
  


template <typename T>
inline T LadderFilter<T>::getGfromNormFreq(T nf) const
{
    nf *= (1.0 / oversampleRate);
    const T g2 = NonUniformLookupTable<T>::lookup(*fs2gLookup, nf);
    return g2;
}

template <typename T>
inline void LadderFilter<T>::setNormalizedFc(T input)
{
    if (input == lastNormalizedFc) {
        return;
    }
    lastNormalizedFc = input;
    _g = getGfromNormFreq(input);
    updateFilter();
    updateFeedback();
}

template <typename T>
void LadderFilter<T>::setBassMakeupGain(T g)
{
    assert(g >= 1);     // must be
    assert(g < 10);     // tends to be
    if (g != bassMakeupGain) {
        bassMakeupGain = g;
        dump("setBassG");
    }
}

template <typename T>
void LadderFilter<T>::setGain(T g)
{
    if (g != gain) {
        gain = g;
        dump("set gain");
    }
}

template <typename T>
void LadderFilter<T>::setFreqSpread(T s)
{
    if (s == freqSpread) {
        return;
    }
    freqSpread = s;

    s *= .5;        // cut it down
    assert(s <= 1 && s >= 0);

    T s2 = s + 1;       // 1..2
    AudioMath::distributeEvenly(stageFreqOffsets, 4, s2);

    updateFilter();
}

template <typename T>
void LadderFilter<T>::updateFilter()
{
    for (int i = 0; i < 4; ++i) {
        stageG[i] = _g * stageFreqOffsets[i];
    }

    if (bypassFirstStage) {
        stageG[0] = getGfromNormFreq(T(.9));
    }

    dump("update");
}

template <typename T>
void LadderFilter<T>::setEdge(T e)
{
    if (e == rawEdge) {
        return;
    }
    rawEdge = e;
    assert(e <= 1 && e >= 0);

    T e2 = 1;
    if (e > .5) {
        e2 = 6 * (e - T(.5)) + 1;
    } else {
        e2 = e * T(1.75) + T(.125);
    }
    processedEdge = e2;
    updateStageGains();   
    dump("set edge");
}

template <typename T>
void LadderFilter<T>::updateStageGains()
{
    // turn off the edge if not 4p lowpass filter
    T edgeToUse = processedEdge;
    switch (type) {
        case Types::_4PLP:
      //  case Types::_3PLP:
      //  case Types::_2PLP:
      //  case Types::_1PLP:
            edgeToUse = processedEdge;
            break;
        default:
            edgeToUse = 1;
    }
     AudioMath::distributeEvenly(stageGain, 4, edgeToUse);
}


template <typename T>
void LadderFilter<T>::setVoicing(Voicing v)
{
    voicing = v;
}

template <typename T>
void LadderFilter<T>::setType(Types t)
{
    if (t == type)
        return;

    bypassFirstStage = false;
    type = t;
    switch (type) {
        case Types::_4PLP:
            stageTaps[3] = 1;
            stageTaps[2] = 0;
            stageTaps[1] = 0;
            stageTaps[0] = 0;
            break;
        case Types::_3PLP:
            stageTaps[3] = 0;
            stageTaps[2] = 1;
            stageTaps[1] = 0;
            stageTaps[0] = 0;
            break;
        case Types::_2PLP:
            stageTaps[3] = 0;
            stageTaps[2] = 0;
            stageTaps[1] = 1;
            stageTaps[0] = 0;
            break;
        case Types::_1PLP:
            stageTaps[3] = 0;
            stageTaps[2] = 0;
            stageTaps[1] = 0;
            stageTaps[0] = 1;
            break;
        case Types::_2PBP:
            stageTaps[3] = 0;
            stageTaps[2] = 0;
            stageTaps[1] = T(-.68) * 2;
            stageTaps[0] = T(.68) * 2;
            break;
        case Types::_2HP1LP:
            stageTaps[3] = 0;
            stageTaps[2] = T(.68) * 2;
            stageTaps[1] = T(-1.36) * 2;
            stageTaps[0] = T(.68) * 2;
            break;
        case Types::_3HP1LP:
            stageTaps[3] = T(-.68) * 4;
            stageTaps[2] = T(2.05) * 4;
            stageTaps[1] = T(-2.05) * 4;
            stageTaps[0] = T(.68) * 4;
            break;
        case Types::_3PHP:
            bypassFirstStage = true;
            stageTaps[3] = -1;
            stageTaps[2] = 3;
            stageTaps[1] = -3;
            stageTaps[0] = 1;
            break;
        case Types::_2PHP:
            bypassFirstStage = true;
            stageTaps[3] = 0;
            stageTaps[2] = 1;
            stageTaps[1] = -2;
            stageTaps[0] = 1;
            break;
        case Types::_1PHP:
            bypassFirstStage = true;
            stageTaps[3] = 0;
            stageTaps[2] = 0;
            stageTaps[1] = -1;
            stageTaps[0] = 1;
            break;
        case Types::_4PBP:
            stageTaps[3] = T(-.68) * 4;
            stageTaps[2] = T(1.36) * 4;
            stageTaps[1] = T(-.68) * 4;
            stageTaps[0] = 0;
            break;
        case Types::_1LPNotch:
            stageTaps[3] = 0;
            stageTaps[2] = T(1.36);
            stageTaps[1] = T(-1.36);
            stageTaps[0] = T(.68);
            break;
        case Types::_3AP1LP:
            stageTaps[3] = T(-2.73);
            stageTaps[2] = T(4.12);
            stageTaps[1] = T(-2.05);
            stageTaps[0] = T(.68);
            break;
        case Types::_NOTCH:
            bypassFirstStage = true;
            stageTaps[3] = 0;
            stageTaps[2] = 2;
            stageTaps[1] = -2;
            stageTaps[0] = 1;
            break;
        case Types::_PHASER:
            bypassFirstStage = true;
            stageTaps[3] = -4;
            stageTaps[2] = 6;
            stageTaps[1] = -3;
            stageTaps[0] = 1;
            break;
        default:
            assert(false);
    }
    updateFilter();
    updateSlope();
    updateStageGains();         // many filter types turn off the edge
    dump("set type");
}

template <typename T>
inline T LadderFilter<T>::getOutput()
{
    return mixedOutput * 5 * bassMakeupGain;
}

template <typename T>
inline void LadderFilter<T>::setFeedback(T f)
{
    if (f == requestedFeedback) {
        return;
    }
    requestedFeedback = f;
    updateFeedback();
    dump("feedback");
}

template <typename T>
inline void LadderFilter<T>::updateFeedback()
{
    double maxFeedback = 4;
    double fNorm = lastNormalizedFc;

    // Becuase this filter isn't zero delay, it can get unstable at high freq.
    // So limite the feedback up there.
    if (fNorm <= .002) {
        maxFeedback = 3.99;
    } else if (fNorm <= .008) {
        maxFeedback = 3.9;
    } else if (fNorm <= .032) {
        maxFeedback = 3.8;
    } else if (fNorm <= .064) {
        maxFeedback = 3.6;
    } else if (fNorm <= .128) {
        maxFeedback = 2.95;
    } else if (fNorm <= .25) {
        maxFeedback = 2.85;
    } else {
        maxFeedback = 2.30;
    }

    assert(requestedFeedback <= 4 && adjustedFeedback >= 0);

    adjustedFeedback = std::min(requestedFeedback, (T) maxFeedback);
    adjustedFeedback = std::max(adjustedFeedback, T(0));
}




template <typename T>
inline void LadderFilter<T>::run(T input)
{
    input *= gain;
    float buffer[oversampleRate];
    up.process(buffer, (float) input);

    switch (voicing) {
        case Voicing::Classic:
            runBufferClassic(buffer, oversampleRate);
            break;
        case Voicing::Clip2:
            runBufferClip2(buffer, oversampleRate);
            break;
        case Voicing::Fold:
            runBufferFold(buffer, oversampleRate);
            break;
        case Voicing::Fold2:
            runBufferFold2(buffer, oversampleRate);
            break;
        case Voicing::Clean:
            runBufferClean(buffer, oversampleRate);
            break;
        default:
            assert(false);
    }
    mixedOutput = down.process(buffer) * finalVolume;
}

/**************************************************************************************
 *
 * A set of macros for (ugh) building up process functions for different distortion functions
 */
#define PROC_PREAMBLE(name) template <typename T> \
    inline void  LadderFilter<T>::name(float* buffer, int numSamples) { \
        for (int i = 0; i < numSamples; ++i) { \
            const T input = buffer[i]; \
            T temp = input - adjustedFeedback * stageOutputs[3]; \
            temp = std::max(T(-3), temp); \
            temp = std::min(T(3), temp);

#define PROC_END \
        temp = 0; \
        for (int i = 0; i < 4; ++i) { \
            temp += stageOutputs[i] * stageTaps[i]; \
        } \
        temp = std::max(T(-1.7), temp); \
        temp = std::min(T(1.7), temp); \
        buffer[i] = float(temp); \
     } \
}

#define ONETAP(func, index) \
    temp *= stageGain[index]; \
    func(); \
    temp = lpfs[index].run(temp, stageG[index]); \
    stageOutputs[index] = temp;

#define BODY( func0, func1, func2, func3) \
    ONETAP(func0, 0) \
    ONETAP(func1, 1) \
    ONETAP(func2, 2) \
    ONETAP(func3, 3)

#define TANH() temp = T(2) * LookupTable<T>::lookup(*tanhLookup.get(), T(.5) * temp, true)

#define CLIP() temp = std::max<T>(temp, -1.f); temp = std::min<T>(temp, 1.f)
#define CLIP_TOP()  temp = std::min<T>(temp, 1.f)
#define CLIP_BOTTOM()  temp = std::max<T>(temp, -1.f)

#define FOLD() temp = AudioMath::fold(float(temp))
#define FOLD_ATTEN() temp = AudioMath::fold(float(temp) * .5f)
//#define FOLD_ATTEN() temp = AudioMath::fold(float(temp) * 1)
#define FOLD_BOOST() temp = 2 * AudioMath::fold(float(temp))

#define FOLD_TOP() temp = (temp > 0) ? (T) AudioMath::fold(float(temp)) : temp
#define FOLD_BOTTOM() temp = (temp < 0) ? (T) AudioMath::fold(float(temp)) : temp
#define NOPROC()

//#define TRIODE1() temp = T(1.4) * shaper.lookup(float(temp * .4f), 7)
//#define TRIODE2_ATTEN() temp = T(.1) * shaper.lookup(float(temp), 5)
//#define TRIODE2() temp = 1.1 * shaper.lookup(float(temp * .4f), 5)
//#define TRIODE2g() temp = 3 * shaper.lookup(float(temp * .4f), 5)
//#define TRIODE2b() temp = 1.1 * -shaper.lookup(float(-temp * .4f), 5)

PROC_PREAMBLE(runBufferClassic)
BODY(TANH, TANH, TANH, TANH)
PROC_END

PROC_PREAMBLE(runBufferClip2)
BODY(CLIP_TOP, CLIP_BOTTOM, CLIP_TOP, CLIP_BOTTOM)
PROC_END

PROC_PREAMBLE(runBufferFold)
BODY(FOLD_ATTEN, FOLD, FOLD, FOLD)
PROC_END

PROC_PREAMBLE(runBufferFold2)
BODY(FOLD_TOP, FOLD_BOTTOM, FOLD_TOP, FOLD_BOTTOM)
PROC_END

PROC_PREAMBLE(runBufferClean)
BODY(NOPROC, NOPROC, NOPROC, NOPROC)
PROC_END

template <typename T>
inline  std::vector<std::string> LadderFilter<T>::getTypeNames()
{
    return {
        "4P LP",
        "3P LP",
        "2P LP",
        "1P LP",
        "2P BP",
        "2HP+1LP",
        "3HP+1LP",
        "4P BP",
        "1LP+Notch",
        "3AP+1LP",
        "3P HP",
        "2P HP",
        "1P HP",
        "NOTCH",
        "PHASER"
    };
}

template <typename T>
inline  std::vector<std::string> LadderFilter<T>::getVoicingNames()
{
    return {
        "Transistor",
        "Asym Clip",
        "Fold",
        "Asym Fold",
        "Clean"
    };
}
  
#if 0
template <typename T>
inline void LadderFilter<T>::runBufferClean(float* buffer, int numSamples)
{

    for (int i = 0; i < numSamples; ++i) {
        const T input = buffer[i];

        T temp = input - feedback * stageOutputs[3];
        temp = std::max(T(-10), temp); 
        temp = std::min(T(10), temp);

        temp = lpfs[0].run(temp, stageG[0]);
        stageOutputs[0] = temp;

        temp = lpfs[1].run(temp, stageG[1]);
        stageOutputs[1] = temp;

        temp = lpfs[2].run(temp, stageG[2]);
        stageOutputs[2] = temp;

        temp = lpfs[3].run(temp, stageG[3]);

        temp = std::max(T(-10), temp);
        temp = std::min(T(10), temp);
        stageOutputs[3] = temp;

        if (type != Types::_4PLP) {
            temp = 0;
            for (int i = 0; i < 4; ++i) {
                temp += stageOutputs[i] * stageTaps[i];
            }
        }
        buffer[i] = float(temp);
    }
}
#endif

#if 0
template <typename T>
inline void LadderFilter<T>::runBufferClassic(T* buffer, int numSamples)
{
    const float k = 1.f / 5.f;
    const float j = 1.f / k;
    for (int i = 0; i < numSamples; ++i) {
        const T input = buffer[i];
        T temp = input - feedback * stageOutputs[3];
        temp *= stageGain[0];
        temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
        temp = lpfs[0].run(temp, stageG[0]);
        stageOutputs[0] = temp;

        temp *= stageGain[1];
        temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
        temp = lpfs[1].run(temp, stageG[1]);
        stageOutputs[1] = temp;

        temp *= stageGain[2];
        temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
        temp = lpfs[2].run(temp, stageG[2]);
        stageOutputs[2] = temp;

        temp *= stageGain[3];
        temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
        temp = lpfs[3].run(temp, stageG[3]);
        stageOutputs[3] = temp;

        if (type != Types::_4PLP) {
            temp = 0;
            for (int i = 0; i < 4; ++i) {
                temp += stageOutputs[i] * stageTaps[i];
            }
        }
        buffer[i] = temp;
    }

   // mixedOutput = temp;
   // return temp;
}
#endif


