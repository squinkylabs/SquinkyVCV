#include "Actions.h"
#include "ActionContext.h"

#include <assert.h>
#include "MidiSequencer.h"

Actions::Actions() {
    _map = {
        {"help", help},
        {"loop", loop},
        {"change.track.length", changeTrackLength},
        {"insert.default", insertDefault},
        {"insert.whole.advance", insertWholeAdvance},
        {"insert.half.advance", insertHalfAdvance},
        {"insert.quarter.advance", insertQuarterAdvance},
        {"insert.eighth.advance", insertEighthAdvance},
        {"insert.sixteenth.advance", insertSixteenthAdvance},

        {"insert.whole", insertWhole},
        {"insert.half", insertHalf},
        {"insert.quarter", insertQuarter},
        {"insert.eighth", insertEighth},
        {"insert.sixteenth", insertSixteenth},

        {"move.left.normal", moveLeftNormal},
        {"move.right.normal", moveRightNormal},
        {"move.up.normal", moveUpNormal},
        {"move.down.normal", moveDownNormal},

        {"move.left.all", moveLeftAll},
        {"move.right.all", moveRightAll},
        {"move.left.measure", moveLeftMeasure},
        {"move.right.measure", moveRightMeasure},
        {"move.up.octave", moveUpOctave},
        {"move.down.octave", moveDownOctave},

        {"select.next", selectNext},
        {"select.next.extend", selectNextExtend},
        {"select.previous", selectPrevious},
        {"select.previous.extend", selectPreviousExtend},
        {"select.all", selectAll},

        {"value.increment.small", valueIncrementSmall},
        {"value.increment.normal", valueIncrementNormal},
        {"value.increment.large", valueIncrementLarge},
        {"value.decrement.small", valueDecrementSmall},
        {"value.decrement.normal", valueDecrementNormal},
        {"value.decrement.large", valueDecrementLarge},

        {"cut", cut},
        {"copy", copy},
        {"paste", paste},
        {"edit.start.time", editStartTime},
        {"edit.duration", editDuration},
        {"edit.pitch", editPitch},

        {"grab.default.note", grabDefaultNote},
        {"delete.note", deleteNote}

    };
}

Actions::action Actions::getAction(const std::string& name) {
    auto it = _map.find(name);
    if (it == _map.end()) {
        fprintf(stderr, "bad action name: %s\n", name.c_str());
        return nullptr;
    }

    action a = it->second;
    return a;
}

/********************** Note insert ******************************/

void Actions::handleInsertPresetNote(
    ActionContext& context,
    MidiEditor::Durations duration,
    bool advanceAfter) {
    MidiSequencerPtr sequencer = context.sequencer;
    assert(sequencer);

    // First, see if step record wants this event.
    bool handled = context.handleInsertPresetNote(sequencer, duration, advanceAfter);
    if (!handled) {
        sequencer->editor->insertPresetNote(duration, advanceAfter);
    }
}

void Actions::handleNoteEditorChange(MidiSequencerPtr sequencer, ChangeType type, bool increase) {
    int units = 1;
    bool ticks = false;

    // make >,< always advance cursor by ticks i note not selected
    const bool noteSelected = bool(sequencer->editor->getNoteUnderCursor());
    if ((type == ChangeType::small) && !noteSelected) {
        sequencer->editor->advanceCursor(MidiEditor::Advance::Tick, increase ? 1 : -1);
        return;
    }

    switch (sequencer->context->noteAttribute) {
        case MidiEditorContext::NoteAttribute::Pitch: {
            int semitones = (type == ChangeType::large) ? 12 : 1;
            if (!increase) {
                semitones = -semitones;
            }
            sequencer->editor->changePitch(semitones);
        } break;

        case MidiEditorContext::NoteAttribute::Duration: {
            switch (type) {
                case ChangeType::large:
                    units = 4;
                    ticks = false;
                    break;
                case ChangeType::small:
                    units = 1;
                    ticks = true;
                    break;
                case ChangeType::normal:
                    units = 1;
                    ticks = false;
                    break;
                default:
                    assert(false);
            }
            if (!increase) {
                units = -units;
            }
            sequencer->editor->changeDuration(ticks, units);
        } break;

        case MidiEditorContext::NoteAttribute::StartTime: {
            switch (type) {
                case ChangeType::large:
                    units = 4;
                    ticks = false;
                    break;
                case ChangeType::small:
                    units = 1;
                    ticks = true;
                    break;
                case ChangeType::normal:
                    units = 1;
                    ticks = false;
                    break;
                default:
                    assert(false);
            }
            if (!increase) {
                units = -units;
            }
            sequencer->editor->changeStartTime(ticks, units);
        } break;
    }
}

extern void sequencerHelp();

void Actions::help(ActionContext& context) {
#ifdef _SEQQ
    sequencerHelp();
#endif
}

void Actions::insertDefault(ActionContext& context) {
    context.sequencer->editor->insertDefaultNote(true, false);
}

void Actions::insertHalfAdvance(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Half, true);
}

void Actions::insertWholeAdvance(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Whole, true);
}

void Actions::insertQuarterAdvance(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Quarter, true);
}

void Actions::insertEighthAdvance(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Eighth, true);
}

void Actions::insertSixteenthAdvance(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Sixteenth, true);
}

void Actions::insertHalf(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Half, false);
}

void Actions::insertWhole(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Whole, false);
}

void Actions::insertQuarter(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Quarter, false);
}

void Actions::insertEighth(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Eighth, false);
}

void Actions::insertSixteenth(ActionContext& context) {
    handleInsertPresetNote(context, MidiEditor::Durations::Sixteenth, false);
}
//******************** cursor movement ****************************

void Actions::moveLeftNormal(ActionContext& context) {
    context.sequencer->editor->advanceCursor(MidiEditor::Advance::GridUnit, -1);
}

void Actions::moveRightNormal(ActionContext& context) {
    context.sequencer->editor->advanceCursor(MidiEditor::Advance::GridUnit, 1);
}

void Actions::moveUpNormal(ActionContext& context) {
    context.sequencer->editor->changeCursorPitch(1);
}

void Actions::moveDownNormal(ActionContext& context) {
    context.sequencer->editor->changeCursorPitch(-1);
}

void Actions::moveLeftAll(ActionContext& context) {
    context.sequencer->editor->advanceCursor(MidiEditor::Advance::All, -1);
}

void Actions::moveLeftMeasure(ActionContext& context) {
    context.sequencer->editor->advanceCursor(MidiEditor::Advance::Measure, -1);
}

void Actions::moveRightAll(ActionContext& context) {
    context.sequencer->editor->advanceCursor(MidiEditor::Advance::All, 1);
}

void Actions::moveRightMeasure(ActionContext& context) {
    context.sequencer->editor->advanceCursor(MidiEditor::Advance::Measure, 1);
}

void Actions::moveUpOctave(ActionContext& context) {
    context.sequencer->editor->changeCursorPitch(12);
}

void Actions::moveDownOctave(ActionContext& context) {
    context.sequencer->editor->changeCursorPitch(-12);
}

//********************* select next note ************

void Actions::selectPrevious(ActionContext& context) {
    context.sequencer->editor->selectPrevNote();
}
void Actions::selectPreviousExtend(ActionContext& context) {
    context.sequencer->editor->extendSelectionToPrevNote();
}
void Actions::selectNext(ActionContext& context) {
    context.sequencer->editor->selectNextNote();
}
void Actions::selectNextExtend(ActionContext& context) {
    context.sequencer->editor->extendSelectionToNextNote();
}

void Actions::selectAll(ActionContext& context) {
    context.sequencer->editor->selectAll();
}

//****************** edit note values
void Actions::valueIncrementSmall(ActionContext& context) {
    handleNoteEditorChange(context.sequencer, ChangeType::small, true);
}

void Actions::valueIncrementNormal(ActionContext& context) {
    handleNoteEditorChange(context.sequencer, ChangeType::normal, true);
}

void Actions::valueIncrementLarge(ActionContext& context) {
    handleNoteEditorChange(context.sequencer, ChangeType::large, true);
}

void Actions::valueDecrementSmall(ActionContext& context) {
    handleNoteEditorChange(context.sequencer, ChangeType::small, false);
}

void Actions::valueDecrementNormal(ActionContext& context) {
    handleNoteEditorChange(context.sequencer, ChangeType::normal, false);
}

void Actions::valueDecrementLarge(ActionContext& context) {
    handleNoteEditorChange(context.sequencer, ChangeType::large, false);
}

void Actions::cut(ActionContext& context) {
    context.sequencer->editor->cut();
}

void Actions::copy(ActionContext& context) {
    context.sequencer->editor->copy();
}

void Actions::paste(ActionContext& context) {
    context.sequencer->editor->paste();
}

void Actions::editDuration(ActionContext& context) {
    context.sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Duration);
}

void Actions::editPitch(ActionContext& context) {
    context.sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Pitch);
}

void Actions::editStartTime(ActionContext& context) {
    context.sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::StartTime);
}

void Actions::loop(ActionContext& context) {
    context.sequencer->editor->loop();
}

void Actions::changeTrackLength(ActionContext& context) {
    context.sequencer->editor->changeTrackLength();
}

void Actions::deleteNote(ActionContext& context) {
    context.sequencer->editor->deleteNote();
}

void Actions::grabDefaultNote(ActionContext& context) {
    context.sequencer->editor->grabDefaultNote();
}