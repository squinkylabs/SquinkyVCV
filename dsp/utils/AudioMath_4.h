#pragma once

#include "simd.h"
#include <functional>

class AudioMath_4 
{
public:
    using ScaleFun = std::function<float_4(float_4 cv, float knob, float trim)>;


    /**
     * Generates a scale function for an audio taper attenuverter.
     * Details the same as makeLinearScaler except that the CV
     * scaling will be exponential for most values, becoming
     * linear near zero.
     *
     * Note that the cv and knob will have linear taper, only the
     * attenuverter is audio taper.
     *
     * Implemented with a cached lookup table -
     * only the final scaling is done at run time
     */
    static ScaleFun makeScalerWithBipolarAudioTrim(float x0, float x1, float y0, float y1);

};
