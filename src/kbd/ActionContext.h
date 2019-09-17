#pragma once

#include "MidiEditor.h"
#include "StepRecorder.h"

#include <memory>

class MidiSequencer;


using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;
using StepRecorderPtr = std::shared_ptr<StepRecorder>;

class ActionContext
{
public:
    ActionContext(MidiSequencerPtr s, StepRecorderPtr recorder) : 
        sequencer(s),
        stepRecorder(recorder)
    {

    }
    MidiSequencerPtr sequencer;

    /**
     * returns true if event handled
     */
    bool handleInsertPresetNote(
        MidiSequencerPtr sequencer,
        MidiEditor::Durations duration, 
        bool advanceAfter)
    {
        return stepRecorder->handleInsertPresetNote(sequencer, duration, advanceAfter);
    }
private:
    StepRecorderPtr stepRecorder;
    

};