#pragma once

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

    };
    class Params
    {

    };
    static T run(State&, const Params&);
 

};

template <typename T, int NOsc, int NOut>
inline  T MultiModOsc<T, NOsc, NOut>::run(State&, const Params&)
{
    return 0;
}