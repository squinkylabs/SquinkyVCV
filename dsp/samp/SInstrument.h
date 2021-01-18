#pragma once

#include <memory>

#include "SParse.h"

/**

    parse the file, generate instrument
    look through instrument, find all wave files.
    get loader to load the files

    to play note: let's start easy.
    look through all the regions until we find one that plays. use that
*/

/**
 * An entire instrument.
 * Note that this is used by parser, but it also
 * is used for note playback
 * update: won't user for playback
 */
class SInstrument {
public:

    //---------- part used by parse
    SControl control;
    SGlobal global;

    // Even if there are no groups, we make a dummy one so that data is nicer.
    SGroupList groups;

    bool wasExpanded = false;

private:
    // bool testMode = false;
};
using SInstrumentPtr = std::shared_ptr<SInstrument>;
