
#include "StochasticNote.h"

StochasticNote::StochasticNote(const StochasticDisplayNote& n) : duration(n.timeDuration()) {
}

bool StochasticNote::operator < (const StochasticNote& other) const {
    return other.duration < this->duration;
}