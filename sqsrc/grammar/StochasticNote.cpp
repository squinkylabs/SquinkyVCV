
#include "StochasticNote.h"

#include <assert.h>

// TODO: make better
const static int _ppq = 96;
const int StochasticNote::ppq =_ppq;

bool StochasticNote::operator<(const StochasticNote& other) const {
    return other.duration < this->duration;
}

static const int half_dur = _ppq * 2;
static const int quarter_dur = _ppq;
static const int eighth_dur = _ppq / 2;

StochasticNote StochasticNote::half() {
    return StochasticNote(half_dur);
}

StochasticNote StochasticNote::quarter() {
    return StochasticNote(quarter_dur);
}

StochasticNote StochasticNote::eighth() {
    return StochasticNote(eighth_dur);
}

std::string StochasticNote::toText() const {
    std::string ret;
    switch(duration) {
        case half_dur:
            ret = "Half";
            break;
        case quarter_dur:
            ret = "Quarter";
            break;
        case eighth_dur:
            ret = "Eighth";
            break;
        default:
            assert(false);
    }
    return ret;
}