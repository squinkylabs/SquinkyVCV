#include "ISeqSettings.h"
#include "MidiSequencer.h"
#include "StepRecorder.h"
#include "WidgetComposite.h"

#include <assert.h>

void StepRecorder::onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer)
{
  RecordInputData data;
    bool isData = seqComp->poll(&data);
    if (isData) {
        switch (data.type) {
            case RecordInputData::Type::noteOn:
                onNoteOn(data.pitch, sequencer);
                break;
            case RecordInputData::Type::allNotesOff:
                onAllNotesOff(sequencer);
                break;
            default:
                assert(false);
        }
    }
}

void StepRecorder::onNoteOn(float pitchCV, MidiSequencerPtr sequencer)
{
    if (numNotesActive == 0) {
        // first note in a new step.
        // clear selection and get the default advance time
        sequencer->selection->clear();
        advanceTime = sequencer->editor->getAdvanceTimeAfterNote();;
    }
    const float time = sequencer->context->cursorTime();
    sequencer->editor->moveToTimeAndPitch(time, pitchCV);

    // don't advance after, but do extend selection
    sequencer->editor->insertDefaultNote(false, true);
    MidiNoteEventPtr note = sequencer->editor->getNoteUnderCursor();
    assert(note);

    if (note) {
        // not needed
        sequencer->selection->addToSelection(note, true);
    }
    lastPitch = pitchCV;
    ++numNotesActive;  
}

void StepRecorder::onAllNotesOff(MidiSequencerPtr sequencer)
{
   // float advanceTime = sequencer->editor->getAdvanceTimeAfterNote();
    float time = sequencer->context->cursorTime();

    time += advanceTime;
    sequencer->editor->moveToTimeAndPitch(time, lastPitch);
    numNotesActive = 0;
}

bool StepRecorder::handleInsertPresetNote(
    MidiSequencerPtr sequencer,
    MidiEditor::Durations duration, 
    bool advanceAfter)
{
    if (!isActive()) {
        return false;
    }
    // 
    const float artic = sequencer->context->settings()->articulation();
    advanceTime = MidiEditor::getDuration(duration);
    float finalDuration =  advanceTime * artic;
    sequencer->editor->setDuration(finalDuration);
    return true;
}

bool StepRecorder::isActive() const 
{
    return numNotesActive > 0;
}
