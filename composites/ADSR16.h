/**
 * A simple 4 channel ADSR, based on the VCV Fundamental ADSR
 */

#pragma once

#include "simd.h"

class ADSR16
{
public:
    /** All times in milliseocnds.
     *  sustain is 0..1
     * 
     */
    void setA(float attacksMilliseconds);
    void setD(float decaysMilliseconds);
    void setS(float sustains);
    void setR(float releasesMilliseconds);

    /* v > 1 = on
     */
    //setGate(float_4 gates);
    void step(float_4 gates0, float_4 gates1, float_4 gates2, float_4 gates3);
    float_4 env[4] = {0.f};
private:
	float_4 attacking [4] = {float_4::zero()};
	
	float_4 attackLambda[4] = {0.f};
	float_4 decayLambda[4] = {0.f};
	float_4 releaseLambda[4] = {0.f};
	float_4 sustain[4] = {0.f};
};

inline void  ADSR16::step(float_4 gates0, float_4 gates1, float_4 gates2, float_4 gates3)
{ 
}

inline void ADSR16::setA(float attacksMilliseconds)
{

}

inline void ADSR16::setD(float attacksMilliseconds)
{
    
}

inline void ADSR16::setS(float attacksMilliseconds)
{
    
}

inline void ADSR16::setR(float attacksMilliseconds)
{
    
}

