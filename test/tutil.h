#pragma once

#include <functional>
#include <tuple>

// TODO: move these to utils
template <class T>
inline void initComposite(T& comp)
{
    comp.init(); 
    auto icomp = comp.getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        comp.params[i].value = param.def;
    }
}

/**
 * returns mix:min:average
 */
inline std::tuple<float, float, float> getSignalStats(int iterations, std::function<float(void)> lambda)
{
    float positive = -100;
    float negative = 100; 
    float sum = 0; 
  //  const int iterations = 10000;  
    for (int i=0; i < iterations; ++i) {  
     //   wvco.step();
      //  float x = wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(0); 
        float x = lambda();
        sum += x;
        positive = std::max(positive, x); 
        negative = std::min(negative, x);  
    } 
    return std::make_tuple(negative, positive, sum / iterations);
}