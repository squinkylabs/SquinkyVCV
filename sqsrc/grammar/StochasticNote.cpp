
#include "StochasticNote.h"
#include <assert.h>

// TODO: make better
const int StochasticNote::ppq = 96;

#if 0       // doesn't work for some reason
StochasticNote::StochasticNote(const StochasticDisplayNote& n) : duration(n.timeDuration()) {
    assert(n.timeDuration() > 0);
    duration = n.timeDuration();
    assert(this->duration > 0);
}
#endif

bool StochasticNote::operator < (const StochasticNote& other) const {
    return other.duration < this->duration;
}