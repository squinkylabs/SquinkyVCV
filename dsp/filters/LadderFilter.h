#pragma once

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
        _3AP1LP
    };

    enum class Voicing
    {
        Classic,
        Clip,
        Clip2,
        Fold,
        Fold2,
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
   
    static std::vector<std::string> getTypeNames();
    static std::vector<std::string> getVoicingNames();
private:
    TrapezoidalLowpass<T> lpfs[4];

    /**
     * Lowpass pole gain
     */
    T _g = .001f;
    T mixedOutput = 0;
    T feedback = 0;
    T gain = T(.3);
    T stageOutputs[4];
    T edge = 0;
    T freqSpread = 0;
    T stageTaps[4] = {0, 0, 0, 1};
    T stageGain[4] = {1, 1, 1, 1};
    T stageFreqOffsets[4] = {1, 1, 1, 1};
    T stageG[4] = {.001f, .001f, .001f, .001f};
    Types type = Types::_4PLP;
    Voicing voicing = Voicing::Classic;
    T lastNormalizedFc = T(.0001);

    std::shared_ptr<NonUniformLookupTableParams<T>> fs2gLookup = makeTrapFilter_Lookup<T>();
    std::shared_ptr<LookupTableParams<float>> tanhLookup = ObjectCache<float>::getTanh5();
    std::shared_ptr<LookupTableParams<T>> expLookup = ObjectCache<T>::getExp2();

    static const int oversampleRate = 4;
    IIRUpsampler up;
    IIRDecimator down;

    void runBufferClassic(T* buffer, int );
    void runBufferClip(T* buffer, int);
    void runBufferClip2(T* buffer, int);
    void runBufferFold(T* buffer, int);
    void runBufferFold2(T* buffer, int);

    void updateFilter();

};

template <typename T>
LadderFilter<T>::LadderFilter()
{
    // fix at 4X oversample
    up.setup(oversampleRate);
    down.setup(oversampleRate);
}

template <typename T>
void LadderFilter<T>::setGain(T g)
{
    gain = g;
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
#if 0
    for (int i = 0; i < 4; ++i) {
        printf("s=%.2f, delta[%d] = %f\n", s, i, stageFreqOffsets[i]);
    }
    fflush(stdout);
#endif
    updateFilter();
}


template <typename T>
void LadderFilter<T>::updateFilter()
{
    for (int i = 0; i < 4; ++i) {
        stageG[i] = _g * stageFreqOffsets[i];
    }
}

template <typename T>
void LadderFilter<T>::setEdge(T e)
{
    if (e == edge) {
        return;
    }
    edge = e;
    assert(e <= 1 && e >= 0);

    T e2 = 1;
    if (e > .5) {
        e2 = 6 * (e - T(.5)) + 1;
    } else {
        e2 = e * T(1.75) + T(.25);
    }
    AudioMath::distributeEvenly(stageGain, 4, e2);
#if 0
    for (int i = 0; i < 4; ++i) {
        printf("e=%.2f, gain[%d] = %f\n", e, i, stageGain[i]);
    }
    fflush(stdout);
#endif

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
            stageTaps[1] = -1;
            stageTaps[0] = 1;
            break;
        case Types::_2HP1LP:
            stageTaps[3] = 0;
            stageTaps[2] = 0;
            stageTaps[1] = -1;
            stageTaps[0] = .5;
            break;
        case Types::_3HP1LP:
            stageTaps[3] = -T(.33334);
            stageTaps[2] = 1;
            stageTaps[1] = -1;
            stageTaps[0] = T(.33334);
            break;
        case Types::_4PBP:
            stageTaps[3] = -.5;
            stageTaps[2] = 1;
            stageTaps[1] = -.5;
            stageTaps[0] = 0;
            break;
        case Types::_1LPNotch:
            stageTaps[3] = 0;
            stageTaps[2] = 1;
            stageTaps[1] = -1;
            stageTaps[0] = .5;
            break;
        case Types::_3AP1LP:
            stageTaps[3] = -T(.6667);
            stageTaps[2] = 1;
            stageTaps[1] = -1;
            stageTaps[0] = .5;
            break;
        default:
            assert(false);
    }
}

template <typename T>
inline T LadderFilter<T>::getOutput() 
{
    return mixedOutput;
}

template <typename T>
inline void LadderFilter<T>::setFeedback(T f)
{
    feedback = f;
}


//step 1: runSampleXXX -> runBufferXXX(T * buffer, size)

template <typename T>
inline void LadderFilter<T>::run(T input)
{
    input *= gain;
    T buffer[oversampleRate];
    up.process(buffer, input);

    switch (voicing) {
        case Voicing::Classic:
            runBufferClassic(buffer, oversampleRate);
            break;

        case Voicing::Clip:
            runBufferClip(buffer, oversampleRate);
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
        default:
            assert(false);
    }

    mixedOutput = down.process(buffer);
    mixedOutput = std::max(T(-10), mixedOutput);
    mixedOutput = std::min(T(10), mixedOutput);
}

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

/**************************************************************************************
 *
 * A set of macros for (ugh) building up process functions for different distortion functions
 */
#define PROC_PREAMBLE(name) template <typename T> \
    inline void  LadderFilter<T>::name(T* buffer, int numSamples) { \
        for (int i = 0; i < numSamples; ++i) { \
            const T input = buffer[i]; \
            T temp = input - feedback * stageOutputs[3];

#define PROC_END \
        if (type != Types::_4PLP) { \
            temp = 0; \
            for (int i = 0; i < 4; ++i) { \
                temp += stageOutputs[i] * stageTaps[i]; \
            } \
        } \
            buffer[i] = temp; \
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
    ONETAP(func3, 3) \

#define TANH() temp = T(5) * LookupTable<float>::lookup(*tanhLookup.get(), T(.2) * temp, true)
#define CLIP() temp = std::max(temp, -1.f); temp = std::min(temp, 1.f)
#define CLIP_TOP()  temp = std::min(temp, 1.f)
#define CLIP_BOTTOM()  temp = std::max(temp, -1.f)
#define FOLD() temp = AudioMath::fold(temp)
#define FOLD_TOP() temp = (temp > 0) ? AudioMath::fold(temp) : temp
#define FOLD_BOTTOM() temp = (temp < 0) ? AudioMath::fold(temp) : temp


PROC_PREAMBLE(runBufferClassic)
BODY( TANH, TANH, TANH, TANH)
PROC_END

PROC_PREAMBLE(runBufferClip)
BODY(CLIP, CLIP, CLIP, CLIP)
PROC_END

PROC_PREAMBLE(runBufferClip2)
BODY(CLIP_TOP, CLIP_BOTTOM, CLIP_TOP, CLIP_BOTTOM)
PROC_END

PROC_PREAMBLE(runBufferFold)
BODY(FOLD, FOLD, FOLD, FOLD)
PROC_END

PROC_PREAMBLE(runBufferFold2)
BODY(FOLD_TOP, FOLD_BOTTOM, FOLD_TOP, FOLD_BOTTOM)
PROC_END

#if 0
template <typename T>
inline T LadderFilter<T>::runSampleClip(T input)
{
    T temp = input - feedback * stageOutputs[3];
    temp = std::max(temp, -1.f); temp = std::min(temp, 1.f);
    temp = lpfs[0].run(temp, _g);
    stageOutputs[0] = temp;

    temp = std::max(temp, -1.f); temp = std::min(temp, 1.f);
    temp = lpfs[1].run(temp, _g);
    stageOutputs[1] = temp;

    temp = std::max(temp, -1.f); temp = std::min(temp, 1.f);
    temp = lpfs[2].run(temp, _g);
    stageOutputs[2] = temp;

    temp = std::max(temp, -1.f); temp = std::min(temp, 1.f);
    temp = lpfs[3].run(temp, _g);
    stageOutputs[3] = temp;

    if (type != Types::_4PLP) {
        temp = 0;
        for (int i = 0; i < 4; ++i) {
            temp += stageOutputs[i] * stageTaps[i];
        }
    }


   // mixedOutput = temp;
    return temp;
}


template <typename T>
inline T LadderFilter<T>::runSampleClip2(T input)
{
    T temp = input - feedback * stageOutputs[3];
    temp = std::min(temp, 1.f);
    temp = lpfs[0].run(temp, _g);
    stageOutputs[0] = temp;

    temp = std::max(temp, -1.f);
    temp = lpfs[1].run(temp, _g);
    stageOutputs[1] = temp;

    temp = std::min(temp, 1.f);
    temp = lpfs[2].run(temp, _g);
    stageOutputs[2] = temp;

    temp = std::max(temp, -1.f);
    temp = lpfs[3].run(temp, _g);
    stageOutputs[3] = temp;

    if (type != Types::_4PLP) {
        temp = 0;
        for (int i = 0; i < 4; ++i) {
            temp += stageOutputs[i] * stageTaps[i];
        }
    }

    return temp;
}
#endif


template <typename T>
inline void LadderFilter<T>::setNormalizedFc(T input)
{
    if (input == lastNormalizedFc) {
        return;
    }
    lastNormalizedFc = input;

    input *= (1.0 / oversampleRate);
    const T g2 = NonUniformLookupTable<T>::lookup(*fs2gLookup, input);
    _g = g2;
    updateFilter();
}

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
        "3AP+1LP"
    };
}

template <typename T>
inline  std::vector<std::string> LadderFilter<T>::getVoicingNames()
{
    return {
        "Transistor",
        "Clip",
        "Asym Clip",
        "Fold",
        "Asym Fold"
    };
}

