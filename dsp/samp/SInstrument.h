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

    SHeading global;
    SHeading master;
    SHeading currentControl;        // in aria instruments this values changes as we compiled,      
                                    // as they illegally have multiple control tags
    SHeading currentGroup;          // temporary holding place for group data as we parse.

    // Even if there are no groups, we make a dummy one so that data is nicer.
    SGroupList groups;

    bool wasExpanded = false;

private:
    // bool testMode = false;
};
using SInstrumentPtr = std::shared_ptr<SInstrument>;
