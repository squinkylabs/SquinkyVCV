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
        Clip2
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
    T stageTaps[4] = {0, 0, 0, 1};
    Types type = Types::_4PLP;
    Voicing voicing = Voicing::Classic;

    std::shared_ptr<NonUniformLookupTableParams<T>> fs2gLookup = makeTrapFilter_Lookup<T>();
    std::shared_ptr<LookupTableParams<float>> tanhLookup = ObjectCache<float>::getTanh5();

    static const int oversampleRate = 4;
    IIRUpsampler up;
    IIRDecimator down;

    //void  processBuffer(T* buffer) const
    T runSampleClassic(T);
    T runSampleClip(T);
    T runSampleClip2(T);

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
  //  printf("set gain %f\n", g); fflush(stdout);
    gain = g;
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

    printf("setting type to %d\n", (int) t); fflush(stdout);
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

template <typename T>
inline void LadderFilter<T>::run(T input)
{
    input *= gain;
    T buffer[oversampleRate];
    up.process(buffer, input);
    for (int i = 0; i < oversampleRate; ++i) {
        switch (voicing) {
            case Voicing::Classic:
                buffer[i] = runSampleClassic(buffer[i]);
                break;
            case Voicing::Clip:
                buffer[i] = runSampleClip(buffer[i]);
                break;
            case Voicing::Clip2:
                buffer[i] = runSampleClip2(buffer[i]);
                break;
            default:
                assert(false);
        }
    }
    mixedOutput = down.process(buffer);
    mixedOutput = std::max(T(-10), mixedOutput);
    mixedOutput = std::min(T(10), mixedOutput);
}

template <typename T>
inline T LadderFilter<T>::runSampleClassic(T input)
{
    const float k = 1.f / 5.f;
    const float j = 1.f / k;

    T temp = input - feedback * stageOutputs[3];
    temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = lpfs[0].run(temp, _g);

    temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    stageOutputs[0] = temp;
    temp = lpfs[1].run(temp, _g);

    temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    stageOutputs[1] = temp;
    temp = lpfs[2].run(temp, _g);

    temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    stageOutputs[2] = temp;
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
inline T LadderFilter<T>::runSampleClip(T input)
{
   // const float k = 1.f / 5.f;
  //  const float j = 1.f / k;

    T temp = input - feedback * stageOutputs[3];
   // temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = std::max(temp, -1.f); temp = std::min(temp, 1.f);
    temp = lpfs[0].run(temp, _g);

  //  temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = std::max(temp, -1.f); temp = std::min(temp, 1.f);
    stageOutputs[0] = temp;
    temp = lpfs[1].run(temp, _g);

  //  temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = std::max(temp, -1.f); temp = std::min(temp, 1.f);
    stageOutputs[1] = temp;
    temp = lpfs[2].run(temp, _g);

  //  temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = std::max(temp, -1.f); temp = std::min(temp, 1.f);
    stageOutputs[2] = temp;
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
   // const float k = 1.f / 5.f;
  //  const float j = 1.f / k;

    T temp = input - feedback * stageOutputs[3];
   // temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = std::min(temp, 1.f);
    temp = lpfs[0].run(temp, _g);

  //  temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = std::max(temp, -1.f);
    stageOutputs[0] = temp;
    temp = lpfs[1].run(temp, _g);

  //  temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = std::min(temp, 1.f);
    stageOutputs[1] = temp;
    temp = lpfs[2].run(temp, _g);

  //  temp = j * LookupTable<float>::lookup(*tanhLookup.get(), k * temp, true);
    temp = std::max(temp, -1.f);
    stageOutputs[2] = temp;
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
inline void LadderFilter<T>::setNormalizedFc(T input)
{
    input *= (1.0 / oversampleRate);
    const T g2 = NonUniformLookupTable<T>::lookup(*fs2gLookup, input);
    _g = g2;
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
        "Asym Clip"
    };
}

/*
#define BX  x = 5

#define AX  a = 3 * a; \
            BX; \
            a = 3 * a

void __foo2()
{
    float a = 1;
    float x = 2;
    AX;
}


#define CX(Z)  a = 3 * a; \
            Z; \
            a = 3 * a

void __foo3()
{
    float a = 1;
    float x = 2;
    CX(BX);
}
*/