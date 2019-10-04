#pragma once

#include "Seq.h"
#include <memory>
class KeyMapping;
class MidiSequencer;
class WidgetComposite;
class StepRecorder;

using KeyMappingPtr = std::shared_ptr<KeyMapping>;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;
using StepRecorderPtr =  std::shared_ptr<StepRecorder>;

class KbdManager
{
public:
    KbdManager();
    bool handle(MidiSequencerPtr sequencer, unsigned key, unsigned mods);
    void onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer);
    bool shouldGrabKeys() const;

private:
    StepRecorderPtr stepRecorder;

    static void init();
    static KeyMappingPtr defaultMappings;
    static KeyMappingPtr userMappings;
};

using KbdManagerPtr = std::shared_ptr<KbdManager>;
