#pragma once

class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

class MidiKeyboardHandler
{
public:
    /**
     * returns true if this key implements repeat
     */
    static bool doRepeat(unsigned key);
    static bool handle(MidiSequencerPtr sequencer, unsigned key, unsigned mods);
private:
    enum class ChangeType { lessThan, plus, bracket };
   
    static void handleNoteEditorChange(MidiSequencerPtr sequencer, ChangeType type, bool increase);
};
