#pragma once

#include "SamplerPlayback.h"
#include <assert.h>

/**
 * a node in the instrument tree that can branch based on pitch
 */
class PitchSwitch : public ISamplerPlayback
{
public:
    void play(VoicePlayInfo&, int midiPitch, int midiVelocity) override;
    void _setTestMode() { testMode = true; }
private:
    bool testMode = false;
};

inline void PitchSwitch::play(VoicePlayInfo&, int midiPitch, int midiVelocity)
{
    assert(false);

    /* old stuff from CompiledInsturment
      if (testMode) {
         info.sampleIndex = 1;
         info.needsTranspose = false;
         info.transposeAmt = 1;
         info.valid = true;
         return;
     }
     info.valid = false;
     auto entry = pitchMap.find(midiPitch);
     if (entry != pitchMap.end()) {
         //assert(false);
         info = *entry->second;         // this requires copy - we could use smart pointers
         
     }
     else printf("pitch %d not found\n", midiPitch)
     */
}