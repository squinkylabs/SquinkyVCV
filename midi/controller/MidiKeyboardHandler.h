#pragma once

#include "MidiEditor.h"
#include "Seq.h"
#include "WidgetComposite.h"
#include <memory>

class MidiSequencer;
class WidgetComposite;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

class MidiKeyboardHandler
{
public:
    /**
     * returns true if this key implements repeat
     */
    static bool doRepeat(unsigned key);
    static bool handle(MidiSequencerPtr sequencer, unsigned key, unsigned mods);

    /**
     * Let's put the mouse handlers in here, too
     */
    static void doMouseClick( MidiSequencerPtr sequencer, float time, float pitchCV,
        bool shiftKey, bool ctrlKey);

    static void onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer);
private:
    enum class ChangeType { lessThan, plus, bracket };
   
    static void handleNoteEditorChange(MidiSequencerPtr sequencer, ChangeType type, bool increase);

    static void handleInsertPresetNote(
        MidiSequencerPtr sequencer,
        MidiEditor::Durations,
        bool advanceAfter);

    class StepRecordImp
    {
    public:
        void onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer);
        
        /**
         * returns true if event handled
         */
        bool handleInsertPresetNote(
            MidiSequencerPtr sequencer,
            MidiEditor::Durations duration, 
            bool advanceAfter);
    private:
        void onNoteOn(float pitch, MidiSequencerPtr sequencer);
        void onAllNotesOff(MidiSequencerPtr sequencer);
        bool isActive() const;

        float lastPitch = 0;
        int numNotesActive = 0;
        float advanceTime = 0;
    };

    static StepRecordImp stepRecordImp;
};
