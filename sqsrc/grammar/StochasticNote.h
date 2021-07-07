
#pragma once

class StochasticNote {
public:
    enum class Durations {
        whole,
        doubleWhole,
        half,
        quarter,
        eighth,
        sixteenth,
        thirtysecond
    };
    enum class Tuples {
        none,
        triplet,
        quintuplet,
        setputlet
    };

    StochasticNote(Durations, Tuples);
    StochasticNote(Durations);
    StochasticNote() = delete;

    // these copy constructors and such are ok, but let's
    // disable them unless we need them.
    StochasticNote(const StochasticNote&) = delete;
    const StochasticNote& operator=(const StochasticNote&) = delete;

private:
    const Durations duration;
    const Tuples tuple;
};

inline
StochasticNote::StochasticNote(Durations d, Tuples t) : duration(d), tuple(t) {

}

inline
StochasticNote::StochasticNote(Durations d) : duration(d), tuple(Tuples::none) {

}