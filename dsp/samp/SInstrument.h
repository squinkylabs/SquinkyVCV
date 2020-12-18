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

    parse the file, generate instrument
    look through insturment, find all wave files.
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
    // This stuff will be leaving, and going to CompiledInstrument
    void getInfo(SVoicePlayInfo&, int midiPitch, int midiVelocity);
    void _setTestMode() {
        testMode = true;
    }


    //---------- part used by parse
    SGlobal global;

    // Even if there are no groups, we make a dummy one so that data is nicer.
    SGroupList groups;

private:
    bool testMode = false;
};
using SInstrumentPtr = std::shared_ptr<SInstrument>;
