#pragma once

#include <assert.h>
#include <cmath>
#include <functional>
#include <algorithm>

class AudioMath
{
public:
    AudioMath() = delete;       // we are only static
    static const double Pi;
    static const double Pi_2;       // Pi / 2
    static const double _2Pi;
    static const double Ln2;
    static const double Ln10;
    static const double E;

    static bool closeTo(double x, double y, double tolerance)
    {
        const bool ret = std::abs(x - y) < tolerance;
        return ret;
    }

    static double db(double g)
    {
        return 20 * log(g) / Ln10;
    }

    static double cents(double f1, double f2)
    {
        return 1200 * std::log2(f1 / f2);
    }

    static double acents(double f1, double f2)
    {
        return std::abs(cents(f1, f2));
    }

    static double gainFromDb(double db)
    {
        return std::exp(Ln10 * db / 20.0);
    }

    static float quadraticBipolar(float x)
    {
        float x2 = x * x;
        return (x >= 0.f) ? x2 : -x2;
    }

    /**
     * Returns a function that generates one period of sin for x = {0..1}.
     * Range (output) is -1 to 1.
     *
     * All makeFunc_xxx functions return functions that are not optimized.
     * They do not use lookup tables.
     */
    static std::function<double(double)> makeFunc_Sin();

    /*
     * Returns a function that generates an exponential defined by two points
     * At input = xMin, output will be yMin.
     * At input = xMax, output will be yMax.
     */
    static std::function<double(double)> makeFunc_Exp(double xMin, double xMax, double yMin, double yMax);

    /*
     * Note that x, y are still defined in terms of the exponential, so
     *  if inv = makeFunc_InverseExp(a, b, c, d);
     *    fun =  makeFunc_InverseExp(a, b, c, d);
     * then
     *  inv(func(x)) == x;
     */
    static std::function<double(double)> makeFunc_InverseExp(double xMin, double xMax, double yMin, double yMax);

    /**
     * Returns a function for an "audio taper" attenuator gain.
     * function is pure exponential for x > .25, linear for x < .25
     * At input 1, output is 1
     * At input .25, output is the gain corresponding to adAtten
     * At input 0, the output is zero.
     */
    static std::function<double(double)> makeFunc_AudioTaper(double dbAtten);
    static std::function<double(double)> makeFunc_InverseAudioTaper(double dbAtten);

    /**
     * ScaleFun is a function the combines CV, knob, and trim into a voltage.
     * Typically a ScaleFun is like an "attenuverter", where the trim input
     * is the attenuverter.
     */
    template <typename T>
    using ScaleFun = std::function<T(T cv, T knob, T trim)>;

    /**
     * Create a ScaleFun with the following properties:
     * 1) The values are combined with the typical formula: x = cv * trim + knob;
     * 2) x is clipped between -5 and 5
     * 3) range is then interpolated between y0, and y1.
     *
     * This particular function is used when knobs are -5..5,
     * and CV range is -5..5.
     *
     *
     * Can easily be used to add, clip and scale just a knob an a CV by 
     * passing 1 for the trim param.
     */
    template <typename T>
    static ScaleFun<T> makeLinearScaler(T y0, T y1)
    {
        const T x0 = -5;
        const T x1 = 5;
        const T a = (y1 - y0) / (x1 - x0);
        const T b = y0 - a * x0;
        return [a, b](T cv, T knob, T trim) {
            T x = cv * trim + knob;
            x = std::max<T>(-5.0f, x);
            x = std::min(5.0f, x);
            return a * x + b;
        };
    }

    /**
     * just like makeLinearScaler, but knob domain specified, too
     * CV domain still -5..5
     * trim -1..1
     */
    template <typename T>
    static ScaleFun<T> makeLinearScaler2(T knobx0, T knobx1, T y0, T y1)
    {
        assert(knobx1 > knobx0);
        assert(y1 > y0);

        // ok,here's the equations for this:
        // f(knob, cv) = ak * knob + av * cv + b    // linear eq of two variables
        // f(x0, 0) = y0                    // from requrements
        // f(x1, 0) = y1
        // f(xbar, 5) = y1              // from req. xbar = .5 * (x0 + x1)

        const T ak = (y1 - y0) / (knobx1 - knobx0);
        const T b = y0 - ak * knobx0;
        const double avd = .2 * (y1 - b) - .1 * ak * (knobx1 + knobx0);
        const T av = T(avd);

        return [ak, av, b, y1, y0](T cv, T knob, T trim) {
            T ret =  ak * knob + av * cv * trim + b;
            ret = std::max<T>(ret, y0);
            ret = std::min<T>(ret, y1);
            return ret;
        };
    }


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
    static ScaleFun<float> makeScalerWithBipolarAudioTrim(float y0, float y1);

    /**
    * SimpleScaleFun is a function the combines CV, and knob, into a voltage.
    * Usually in a synth module that has knob and cv, but no trim.
    *
    * cv and knob are -5 to +5. They are summed, and the sum is limited to 5, -5.
    * Then the sum gets an audio taper
    */
    template <typename T>
    using SimpleScaleFun = std::function<T(T cv, T knob)>;

    /**
     * maps +/- 5 volts from cv and knob to y0, y1
     * with an audio taper.
     */
    static SimpleScaleFun<float> makeSimpleScalerAudioTaper(float y0, float y1);

    template <typename T>
    static std::pair<T, T> getMinMax(const T* data, int numSamples)
    {
        T min = 1, max = -1;
        for (int i = 0; i < numSamples; ++i) {
            const T x = data[i];
            min = std::min(min, x);
            max = std::max(max, x);
        }
        return std::pair<T, T>(min, max);
    }

    /**
     * A random number generator function. uniform random 0..1
     */
    using RandomUniformFunc = std::function<float(void)>;

    static RandomUniformFunc random();

    /**
     * Folds numbers between +1 and -1
     */
    static inline float fold(float x)
    {
        float fold;
        const float bias = (x < 0) ? -1.f : 1.f;
        int phase = int((x + bias) / 2.f);
        bool isEven = !(phase & 1);
        if (isEven) {
            fold = x - 2.f * phase;
        } else {
            fold = -x + 2.f * phase;
        }
        return fold;
    }

    /**
     * Input: data is a list of floats/doubles
     * Output all elements are scaled by the same amount, product of all elements is 1
     */
    template <typename T>
    static inline void normalizeProduct(T* data, int n)
    {
        // get the product of all the input
        T prod = 1;
        for (int i = 0; i < n; ++i) {
            prod *= data[i];
        }

        double logProd = std::log(prod);
        double k = std::exp(-logProd / T(n));
        for (int i = 0; i < n; ++i) {
            data[i] *= T(k);
        }
    }

    /**
     * fills data with a list of numbers that are:
     *      exponentially spaced
     *      product is 1
     *      ratio = data[n] / data[n-1]   
     */
    template <typename T>
    static inline void distributeEvenly(T* data, int n, T ratio)
    {
        T x = 1;
        for (int i = 0; i < n; ++i)
        {
            data[i] = x;
            x *= ratio;
        }
        normalizeProduct(data, n);
    }
};
