
#pragma once

class StochasticDisplayNote;
class StochasticNote {
public:
    StochasticNote(int dur) : duration(dur) {}

    // convenience constructor for existing tests
    StochasticNote(const StochasticDisplayNote& n);
    static const int ppq;
    const int duration;

    bool operator < (const StochasticNote& other) const;
private:

};

/**
 * I did this first. Now I won't want to use it as my funcdamental note type.
 * But it might be useful later.
 */
class StochasticDisplayNote {
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

    StochasticDisplayNote(Durations, Tuples);
    StochasticDisplayNote(Durations);
    StochasticDisplayNote() = delete;

  //  bool operator == (const StochasticNote& other) const;
  //  size_t hash()const;

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

inline StochasticDisplayNote::StochasticDisplayNote(Durations d, Tuples t) : duration(d), tuple(t) {
}

inline StochasticDisplayNote::StochasticDisplayNote(Durations d) : duration(d), tuple(Tuples::none) {
}

inline double StochasticDisplayNote::timeDuration() const {
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
