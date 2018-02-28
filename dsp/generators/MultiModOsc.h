#pragma once

#include "SawOscillator.h"

/**
 * A bunch of LFOs at slightly different frequencies added together in different ways.
 * Taken from Bernie Hutchins' ElectroNotes.
 */
template <typename T, int NOsc, int NOut>
class MultiModOsc 
{
public:
    MultiModOsc() = delete;
    /**
     * Make state and params be nested classes so we don't have
     * to type as many template arguments.
     */
    class State
    {
    public:
        friend MultiModOsc;
    private:
        SawOscillatorState<T> states[NOsc];
    };
    class Params
    {
    public:
        friend MultiModOsc;
        Params();
        /**
         * @param rate is -1..1 arbitrary "low frequency" units
         */
        void setRateAndSpread(T rate, T spread, T inverseSampleRate);
    private:
        SawOscillatorParams<T> params[NOsc];
    };
    static void run(T * output, State&, const Params&);
};

template <typename T, int NOsc, int NOut>
inline MultiModOsc<T, NOsc, NOut>::Params::Params()
{
    setRateAndSpread(.5, .5, T(1.0 / 44100));
}

template <typename T, int NOsc, int NOut>
inline void MultiModOsc<T, NOsc, NOut>::Params::setRateAndSpread(T rate, T spread, T inverseSampleRate)
{
    assert(rate >= -1 && rate <= 1);        // just a sanity check
    assert(inverseSampleRate > (1.0 / 200000));
    assert(inverseSampleRate < (1.0 / 2000)); // same

    T BaseRate = 1.0;
    BaseRate *= std::pow<T>(3, rate);       
    const T dNormSpread = spread * T((3.0 / 2.0) + .5);
    for (int i = 0; i < NOsc; i++) {
        T dMult;
        switch (i) {
            case 1:
                dMult = 1 / T(1.11);
                break;
            case 0:
                dMult = 1.0;
                break;
            case 2:
                dMult = T(1.32);
                break;
            case 3:
                dMult = 1.25;
                break;
            case 4:
                dMult = 1.5;
                break;
            default:
                assert(false);
        }
        //const double y = pow(3, mdSpread);
        //dMult *= y;
        dMult -= 1.0;	// norm to 0
        dMult *= dNormSpread;
        dMult += 1.0;

        const T x = BaseRate * dMult;
      //  mVCOs[i].SetFreq(x);
        const T actual = x * inverseSampleRate;
        printf("setting LFO %d to %f, %f\n", i, x, actual);
        SawOscillator<T, false>::setFrequency(params[i], actual);

    }
}

template <typename T, int NOsc, int NOut>
inline void MultiModOsc<T, NOsc, NOut>::run(T* output, State& state, const Params& params)
{
    T modulators[NOsc];
    for (int i = 0; i < NOsc; ++i) {
        modulators[i] = SawOscillator<T, false>::runTri(state.states[i], params.params[i]);
    }
    assert(false);  // need to return something
}