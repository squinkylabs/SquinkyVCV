#pragma once

#include <memory>

#include "SParse.h"

/**
 * An entire instrument.
 * Note that this is create by parser, but it also
 * is the input to the compiler.
 */
class SInstrument {
public:
    SHeadingList headings;
    bool wasExpanded = false;
    void _dump();
};

using SInstrumentPtr = std::shared_ptr<SInstrument>;
