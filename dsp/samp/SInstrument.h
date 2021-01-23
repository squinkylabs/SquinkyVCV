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

    // Parse is going to treat all these tags the same
    // they will get copied into group when group gets made.
    SKeyValueList currentControl;
    SKeyValueList currentGlobal;
    SKeyValueList currentMaster;

    // Even if there are no groups, we make a dummy one so that data is nicer.
    SGroupList groups;

    bool wasExpanded = false;

private:
    // bool testMode = false;
};
using SInstrumentPtr = std::shared_ptr<SInstrument>;
