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
    void addEntry(int pitch, ISamplerPlaybackPtr data) {
        assert(!pitchMap[pitch]);
        pitchMap[pitch] = data;
    }
private:
    bool testMode = false;
    std::vector<ISamplerPlaybackPtr> pitchMap = std::vector<ISamplerPlaybackPtr>(128);
};

inline void PitchSwitch::play(VoicePlayInfo& info, int midiPitch, int midiVelocity)
{
      if (testMode) {
         info.sampleIndex = 1;
         info.needsTranspose = false;
         info.transposeAmt = 1;
         info.valid = true;
         return;
      }

      info.valid = false;
      const ISamplerPlaybackPtr entry = pitchMap[midiPitch];
      if (entry) {
          entry->play(info, midiPitch, midiVelocity);
      }
}

using PitchSwitchPtr = std::shared_ptr<PitchSwitch>;