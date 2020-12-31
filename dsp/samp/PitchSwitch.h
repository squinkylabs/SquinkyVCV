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
    void _dump(int depth) const override;
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

inline void PitchSwitch::_dump(int depth) const {
    indent(depth);
    printf("begin pitch switch %p\n", this);
    for (auto entry : pitchMap) {
        if (entry) {
            entry->_dump(depth + 1);
        }
    }

    indent(depth);
    printf("end pitch switch %p\n", this);
}

using PitchSwitchPtr = std::shared_ptr<PitchSwitch>;