
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
    // StochasticNote(const StochasticNote&) = delete;
    // const StochasticNote& operator=(const StochasticNote&) = delete;

    const Durations duration;
    const Tuples tuple;

    /** half note = 1/2
     * whole note TBD...
     */
    double timeDuration() const;

private:
};

inline StochasticNote::StochasticNote(Durations d, Tuples t) : duration(d), tuple(t) {
}

inline StochasticNote::StochasticNote(Durations d) : duration(d), tuple(Tuples::none) {
}

inline double StochasticNote::timeDuration() const {
    double ret = 0;
    switch (duration) {
        case Durations::half:
            ret = .5;
            break;
        case Durations::quarter:
            ret = .25;
            break;
        case Durations::eighth:
            ret = .125;
            break;
        case Durations::sixteenth:
            ret = .25 * .25;
            break;
        case Durations::thirtysecond:
            ret = .5 * .25 * .25;
            break;
    }
    return ret;
}
