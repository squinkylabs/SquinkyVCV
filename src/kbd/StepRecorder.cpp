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
    // TODO: if we want to stay in loop, this might be a good place to do it.
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
    // now advance the time past the notes we just inserted.
    float time = sequencer->context->cursorTime();
    time += advanceTime;
    sequencer->editor->moveToTimeAndPitch(time, lastPitch);

    // and clear out the selection
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

    // Adjust the duration of all the selected notes to match the preset note
    // that would have been inserted by the preset note command.
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
