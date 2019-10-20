#pragma once

#include "MidiEditor.h"
#include "Seq.h"
#include <memory>

class WidgetComposite;

class StepRecorder
{
public:
    /**
     * called periodically on the UI thread.
     * (actually from widget::step()).
     * Gives us time to to a little processing.
     * 
     * @param seqComp is the composite that can send is note information from the keyboard input.
     * @param sequencer is the sequencer object (data model and editing commands).
     */
    void onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer);
    
    /**
     * We get called before the other handlers, in case we want to eat
     * the insert note command and use it instead to change our durations.
     *
     * Returns true if event handled.
     */
    bool handleInsertPresetNote(
        MidiSequencerPtr sequencer,
        MidiEditor::Durations duration, 
        bool advanceAfter);
private:
    void onNoteOn(float pitch, MidiSequencerPtr sequencer);
    void onAllNotesOff(MidiSequencerPtr sequencer);
    bool isActive() const;
    void adjustForLoop(MidiSequencerPtr sequencer);

    float lastPitch = 0;
    int numNotesActive = 0;
    float advanceTime = 0;
};