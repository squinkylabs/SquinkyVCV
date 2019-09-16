#pragma once

#include <memory>
class KeyMapping;
class MidiSequencer;

using KeyMappingPtr = std::shared_ptr<KeyMapping>;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

class KbdManager
{
public:
   KbdManager();
   bool handle(MidiSequencerPtr sequencer, unsigned key, unsigned mods);
private:
    static void init();
    static KeyMappingPtr defaultMappings;
    static KeyMappingPtr userMappings;
};

using KbdManagerPtr = std::shared_ptr<KbdManager>;
