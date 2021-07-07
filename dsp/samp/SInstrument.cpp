
#include "SInstrument.h"
#include "SqLog.h"


 void SInstrument::_dump() {
     SQINFO("Num Headings = %d", headings.size());
     for (auto h : headings) {
        // SQINFO("--h:");
        // SHeading::dumpKeysAndValues(headings);
         h->_dump();
         
     }
 }
#if 0
 void SInstrument::getInfo(SVoicePlayInfo& info, int midiPitch, int midiVelocity)
 {
    if (!testMode)
        return;

    info.sampleIndex = 1;
    info.needsTranspose = false;
    info.transposeAmt = 1;
    info.valid = true;
 }
#endif