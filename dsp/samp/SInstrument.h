#pragma once

#include "SParse.h"
#include <memory>
class SVoicePlayInfo {
public:
    bool valid = false;
    int sampleIndex = 0;
    bool needsTranspose = false;
    float transposeAmt = 1;
};



/**
 * An entire instrument.
 * Note that this is used by parser, but it also
 * is used for note playback
 */
class SInstrument {
public:
    void getInfo(SVoicePlayInfo&, int midiPitch, int midiVelocity);


    //---------- part used by parse
    SGlobal global;

    // Even if there are no groups, we make a dummy one so that data is nicer.
    SGroupList groups;

};
using SInstrumentPtr = std::shared_ptr<SInstrument>;
