#pragma once

class MidiSequencer;

class MidiKeyboardHandler
{
public:
    /**
     * returns true if this key implements repeat
     */
    static bool doRepeat(unsigned key);
    static bool handle(MidiSequencer* sequencer, unsigned key, unsigned mods);
private:
    enum class ChangeType { lessThan, plus, bracket };
   
    static void handleNoteEditorChange(MidiSequencer* sequencer, ChangeType type, bool increase);
};
