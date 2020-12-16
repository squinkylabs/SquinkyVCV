
#include "SInstrument.h"


 void SInstrument::getInfo(SVoicePlayInfo& info, int midiPitch, int midiVelocity)
 {
    if (!testMode)
        return;

    info.sampleIndex = 1;
    info.needsTranspose = false;
    info.transposeAmt = 1;
    info.valid = true;
 }