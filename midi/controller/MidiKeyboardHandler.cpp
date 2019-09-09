
#include "../Squinky.hpp"
#include "ISeqSettings.h"
#include "MidiKeyboardHandler.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>

#include <assert.h>


#ifdef _SEQ

MidiKeyboardHandler::StepRecordImp MidiKeyboardHandler::stepRecordImp;

 void MidiKeyboardHandler::StepRecordImp::onNoteOn(float pitchCV, MidiSequencerPtr sequencer)
 {
     if (numNotesActive == 0) {
         sequencer->selection->clear();
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

void MidiKeyboardHandler::StepRecordImp::onAllNotesOff(MidiSequencerPtr sequencer)
{
    float advanceTime = sequencer->editor->getAdvanceTimeAfterNote();
    float time = sequencer->context->cursorTime();

    time += advanceTime;
    sequencer->editor->moveToTimeAndPitch(time, lastPitch);
    numNotesActive = 0;
}

void MidiKeyboardHandler::StepRecordImp::onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer)
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
        printf("MidiKeyboardHandler::onUIThread found input\n");
        fflush(stdout);
    }
}

bool MidiKeyboardHandler::StepRecordImp::handleInsertPresetNote(
    MidiSequencerPtr sequencer,
    MidiEditor::Durations duration, 
    bool advanceAfter)
{
    return false;
}

//************************************************************************

void MidiKeyboardHandler::onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer)
{
    stepRecordImp.onUIThread(seqComp, sequencer);
}

bool MidiKeyboardHandler::doRepeat(unsigned key)
{
    bool doIt = false;
    switch(key) {
        case GLFW_KEY_TAB: 
        case GLFW_KEY_KP_ADD:
        case GLFW_KEY_EQUAL:
        case GLFW_KEY_KP_SUBTRACT:
        case GLFW_KEY_LEFT_BRACKET:
        case GLFW_KEY_RIGHT_BRACKET:
        case GLFW_KEY_MINUS:
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_LEFT:
        case GLFW_KEY_UP:
        case GLFW_KEY_DOWN:
        case GLFW_KEY_COMMA:
        case GLFW_KEY_PERIOD:

        // the new cursor keys
        case GLFW_KEY_4:
        case GLFW_KEY_5:
        case GLFW_KEY_6:
        case GLFW_KEY_R:
        case GLFW_KEY_KP_2:
        case GLFW_KEY_KP_4:
        case GLFW_KEY_KP_8:
        case GLFW_KEY_KP_6:
            doIt = true;
    }
    return doIt;
}

void MidiKeyboardHandler::handleNoteEditorChange(
    MidiSequencerPtr sequencer,
    ChangeType type,
    bool increase)
{
    int units = 1;
    bool ticks = false;

    // make >,< always advance cursor by ticks i note not selected
    const bool noteSelected = bool(sequencer->editor->getNoteUnderCursor());
    if ((type == ChangeType::lessThan) && !noteSelected) {
        sequencer->editor->advanceCursor(MidiEditor::Advance::Tick, increase ? 1 : -1);
        return;
    }
    
    switch(sequencer->context->noteAttribute) {
        case MidiEditorContext::NoteAttribute::Pitch:
            {
                int semitones = (type == ChangeType::bracket) ? 12 : 1;
                if (!increase) {
                    semitones = -semitones;
                }
                sequencer->editor->changePitch(semitones);
            }
            break;

         case MidiEditorContext::NoteAttribute::Duration:
            {
                switch(type) {
                    case ChangeType::bracket:
                        units = 4;
                        ticks = false;
                        break;
                    case ChangeType::lessThan:
                        units = 1;
                        ticks = true;
                        break;
                    case ChangeType::plus:
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
            }
            break;

        case MidiEditorContext::NoteAttribute::StartTime:
            {
                switch(type) {
                    case ChangeType::bracket:
                        units = 4;
                        ticks = false;
                        break;
                    case ChangeType::lessThan:
                        units = 1;
                        ticks = true;
                        break;
                    case ChangeType::plus:
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
            }
            break;
    }
}

extern void sequencerHelp();


void MidiKeyboardHandler::handleInsertPresetNote(
    MidiSequencerPtr sequencer,
    MidiEditor::Durations duration, 
    bool advanceAfter)
{
    // First, see if step record wants this event.
    bool handled = stepRecordImp.handleInsertPresetNote(sequencer, duration, advanceAfter);
    if (!handled) {
     sequencer->editor->insertPresetNote(duration, advanceAfter);
    }
}

bool MidiKeyboardHandler::handle(
    MidiSequencerPtr sequencer,
    unsigned key,
    unsigned mods)
{
    bool handled = false;
    const bool shift = (mods & GLFW_MOD_SHIFT);
    const bool ctrl = (mods & RACK_MOD_CTRL);        // was GLFW_MOD_CONTROL
    //const bool alt = (mods && GLFW_MOD_ALT);
   
    switch(key) {
        case GLFW_KEY_F1:
            sequencerHelp();
            handled = true;
            break;
        case GLFW_KEY_TAB: 
            if (!shift) {
                if (ctrl) {
                    sequencer->editor->selectPrevNote();
                } else {
                    sequencer->editor->selectNextNote();
                }
            } else {
                if (ctrl) {
                    sequencer->editor->extendSelectionToPrevNote();
                } else {
                    sequencer->editor->extendSelectionToNextNote();
                }
            }
            handled = true;
            break;
        case GLFW_KEY_KP_ADD:
            handleNoteEditorChange(sequencer, ChangeType::plus, true);
            handled = true;
            break;
        case GLFW_KEY_EQUAL:
            if (shift) {
                handleNoteEditorChange(sequencer, ChangeType::plus, true);
                handled = true;
            }
            break;
        case GLFW_KEY_KP_SUBTRACT:
            handleNoteEditorChange(sequencer, ChangeType::plus, false);
            handled = true;
            break;
        case GLFW_KEY_LEFT_BRACKET:
            handleNoteEditorChange(sequencer, ChangeType::bracket, false);
            handled = true;
            break;
         case GLFW_KEY_RIGHT_BRACKET:
            handleNoteEditorChange(sequencer, ChangeType::bracket, true);
            handled = true;
            break;
        case GLFW_KEY_MINUS:
            if (!shift) {
                handleNoteEditorChange(sequencer, ChangeType::plus, false);
                handled = true;
            }
            break;
        case GLFW_KEY_COMMA:
            handleNoteEditorChange(sequencer, ChangeType::lessThan, false);
            handled = true;
            break;
        case GLFW_KEY_PERIOD:
            handleNoteEditorChange(sequencer, ChangeType::lessThan, true);
            handled = true;
            break;
        case GLFW_KEY_END:
            sequencer->editor->advanceCursor(
                ctrl ? MidiEditor::Advance::All : MidiEditor::Advance::Measure,
                1);  
            handled = true;
            break;
        case GLFW_KEY_HOME:
            sequencer->editor->advanceCursor(
                ctrl ? MidiEditor::Advance::All : MidiEditor::Advance::Measure,
                -1);  
            handled = true;
            break;

        case GLFW_KEY_6:
        case GLFW_KEY_KP_6:
        case GLFW_KEY_RIGHT:
            {
                //one grid space or quater note
                sequencer->editor->advanceCursor(
                    ctrl ? MidiEditor::Advance::Beat : MidiEditor::Advance::GridUnit,
                    1);  
                handled = true;
            }
            break;
        case GLFW_KEY_4:
        case GLFW_KEY_KP_4:
        case GLFW_KEY_LEFT:
            {
                //one grid space or quater note
                sequencer->editor->advanceCursor(
                    ctrl ? MidiEditor::Advance::Beat : MidiEditor::Advance::GridUnit,
                    -1);  
                handled = true;
            }
            break;
         case GLFW_KEY_PAGE_UP:
            sequencer->editor->changeCursorPitch(12);
            handled = true;
            break;
        case GLFW_KEY_PAGE_DOWN:
            sequencer->editor->changeCursorPitch(-12);
            handled = true;
            break;
        case GLFW_KEY_KP_8:
        case GLFW_KEY_5:
        case GLFW_KEY_UP:
            sequencer->editor->changeCursorPitch(1);
            handled = true;
            break;
        case GLFW_KEY_KP_2:
        case GLFW_KEY_R:
        case GLFW_KEY_DOWN:
            sequencer->editor->changeCursorPitch(-1);
            handled = true;
            break;

        // alpha
        case GLFW_KEY_A:
            {
                if (ctrl) {
                    sequencer->editor->selectAll();
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_C:
            {
                if (ctrl) {
                    sequencer->editor->copy();
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_D:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Duration);
                handled = true;
            }
            break;
        case GLFW_KEY_E:
            {
                handleInsertPresetNote(sequencer, MidiEditor::Durations::Eighth, !shift);
              //  sequencer->editor->insertPresetNote(MidiEditor::Durations::Eighth, !shift);
                handled = true;
            }
            break;
        case GLFW_KEY_H:
            {
                handleInsertPresetNote(sequencer, MidiEditor::Durations::Half, !shift);
                handled = true;
            }
            break;
        case GLFW_KEY_L:
            sequencer->editor->loop();
            handled = true;
            break;
        case GLFW_KEY_P:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Pitch);
                handled = true;
            }
            break;
        case GLFW_KEY_Q:
            {
                handleInsertPresetNote(sequencer, MidiEditor::Durations::Quarter, !shift);
                handled = true;
            }
            break;
        case GLFW_KEY_S:
            {
                if (!ctrl) {
                    sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::StartTime);
                } else {
                    handleInsertPresetNote(sequencer, MidiEditor::Durations::Sixteenth, !shift);
                }
                handled = true;
            }
            break;
        case GLFW_KEY_V:
            {
                if (ctrl) {
                    sequencer->editor->paste();
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_W:
            {
                handleInsertPresetNote(sequencer, MidiEditor::Durations::Whole, !shift);
                handled = true;
            }
            break;
        case GLFW_KEY_X:
            {
                if (ctrl) {
                    sequencer->editor->cut();
                    handled = true;
                } else {
                    handleInsertPresetNote(sequencer, MidiEditor::Durations::Sixteenth, !shift);
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_KP_0:
        case GLFW_KEY_INSERT:
        case GLFW_KEY_ENTER:
            {
                sequencer->editor->insertDefaultNote(!shift, false);
                handled = true;
            }
            break;
        case GLFW_KEY_KP_MULTIPLY:
            sequencer->editor->grabDefaultNote();
            handled = true;
            break;
        case GLFW_KEY_8:
            if (shift) {
                 sequencer->editor->grabDefaultNote();
                handled = true;
            }
            break;        
        case GLFW_KEY_BACKSPACE:
        case GLFW_KEY_KP_DECIMAL:
        case GLFW_KEY_DELETE:
            sequencer->editor->deleteNote();
            handled = true;
            break;
        case GLFW_KEY_N:
            handled = true;
            sequencer->editor->changeTrackLength();
            break;
#ifndef __USE_VCV_UNDO
// In VCV 1.0, VCV provides the undo 
        case GLFW_KEY_Z:
            if (ctrl & !shift) {
                handled = true;
                
                if (sequencer->undo->canUndo()) {
                    sequencer->undo->undo(sequencer);
                } 
            } else if (ctrl & shift) {
                handled = true;
                if (sequencer->undo->canRedo()) {
                    sequencer->undo->redo(sequencer);  
                }
            }
            break;
     
         case GLFW_KEY_Y:
            if (ctrl) {
                handled = true;
                if (sequencer->undo->canRedo()) {
                    sequencer->undo->redo(sequencer);
                } 
            }
            break;
#endif
    }
    return handled;
}

void MidiKeyboardHandler::doMouseClick(MidiSequencerPtr sequencer, 
    float time, float pitchCV, bool shiftKey, bool ctrlKey)
{
    if (!ctrlKey) {
        sequencer->editor->selectAt(time, pitchCV, shiftKey);
    } else {
        sequencer->editor->toggleSelectionAt(time, pitchCV);
    }
}
#endif



// Crazy linker problem - to get perf suite to link I need to put this here.
#if !defined(__PLUGIN)
NVGcolor nvgRGB(unsigned char r, unsigned char g, unsigned char b)
{
	return nvgRGBA(r,g,b,255);
}

NVGcolor nvgRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	NVGcolor color;
	// Use longer initialization to suppress warning.
	color.r = r / 255.0f;
	color.g = g / 255.0f;
	color.b = b / 255.0f;
	color.a = a / 255.0f;
	return color;
}
#endif