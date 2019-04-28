#pragma once

#include "LookupTable.h"
#include "ObjectCache.h"
#include "TrapezoidalLowpass.h"

#include <vector>
#include <string>

template <typename T>
class LadderFilter
{
public:
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

    void run(T);
    T getOutput();

    /**
     * input range >0 to < .5
     */
    void setNormalizedFc(T);

    void setFeedback(T f);
    void setType(Types);
   
    static std::vector<std::string> getTypeNames();
private:
    TrapezoidalLowpass<T> lpfs[4];
    T _g = .001f;
    T output = 0;
    T feedback = 0;
    T stageOutputs[4];
    T stageTaps[4] = {0, 0, 0, 1};
    Types type = Types::_4PLP;

    std::shared_ptr<NonUniformLookupTableParams<T>> fs2gLookup = makeTrapFilter_Lookup<T>();
    std::shared_ptr<LookupTableParams<float>> tanhLookup = ObjectCache<float>::getTanh5();
};

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
            stageTaps[1] = 1;
            stageTaps[0] = 1;
            break;
        case Types::_2HP1LP:
            stageTaps[3] = 0;
            stageTaps[2] = 0;
            stageTaps[1] = 1;
            stageTaps[0] = .5;
            break;
        case Types::_3HP1LP:
            stageTaps[3] = T(.33334);
            stageTaps[2] = 1;
            stageTaps[1] = 1;
            stageTaps[0] = T(.33334);
            break;
        case Types::_4PBP:
            stageTaps[3] = .5;
            stageTaps[2] = 1;
            stageTaps[1] = .5;
            stageTaps[0] = 0;
            break;
        case Types::_1LPNotch:
            stageTaps[3] = 0;
            stageTaps[2] = 1;
            stageTaps[1] = 1;
            stageTaps[0] = .5;
            break;
        case Types::_3AP1LP:
            stageTaps[3] = T(.6667);
            stageTaps[2] = 1;
            stageTaps[1] = 1;
            stageTaps[0] = .5;
            break;
        default:
            assert(false);
    }
}

template <typename T>
inline T LadderFilter<T>::getOutput() 
{
    return output;
}

template <typename T>
inline void LadderFilter<T>::setFeedback(T f)
{
    feedback = f;
}

template <typename T>
inline void LadderFilter<T>::run(T input)
{
    const float k = 1.f / 5.f;
    const float j = 1.f / k;

    input = input - feedback * output;
    input = j * LookupTable<float>::lookup(*tanhLookup.get(), k * input, true);
    T temp = lpfs[0].run(input, _g);

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

    temp = std::max(T(-10), temp);
    temp = std::min(T(10), temp);
    output = temp;
}

template <typename T>
inline void LadderFilter<T>::setNormalizedFc(T input)
{
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
