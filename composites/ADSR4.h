/**
 * A simple 4 channel ADSR, based on the VCV Fundamental ADSR
 */

#pragma once

#include "simd.h"

class ADSR4
{
public:
    /** All times in milliseocnds.
     *  sustain is 0..1
     * 
     */
    void setA(float_4 attacksMilliseconds);
    void setD(float_4 decaysMilliseconds);
    void setS(float_4 sustains);
    void setR(float_4 releasesMilliseconds);

    /* v > 1 = on
     */
    //setGate(float_4 gates);
    float_4 step(float_4 gates);
};

inline float_4 ADSR4::step(float_4 gates)
{
    return 0; 
}

inline void ADSR4::setA(float_4 attacksMilliseconds)
{

}

inline void ADSR4::setD(float_4 attacksMilliseconds)
{
    
}

inline void ADSR4::setS(float_4 attacksMilliseconds)
{
    
}

inline void ADSR4::setR(float_4 attacksMilliseconds)
{
    
}

