#pragma once

#include "MidiEditor.h"
#include "Seq.h"
#include <memory>

class WidgetComposite;

class StepRecorder
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