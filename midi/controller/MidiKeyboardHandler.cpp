
#include "../Squinky.hpp"
#include "MidiKeyboardHandler.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>

#include <assert.h>

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

#ifdef _SEQ

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

bool MidiKeyboardHandler::handle(
    MidiSequencerPtr sequencer,
    unsigned key,
    unsigned mods)
{
    bool handled = false;
    const bool shift = (mods & GLFW_MOD_SHIFT);
    const bool ctrl = (mods & GLFW_MOD_CONTROL);
   
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
        case GLFW_KEY_RIGHT:
            {
                int units = ctrl ? 4 : 1;
                sequencer->editor->advanceCursor(false, units);
                handled = true;
            }
            break;
        case GLFW_KEY_LEFT:
            {
                int units = ctrl ? -4 : -1;
                sequencer->editor->advanceCursor(false, units);
                handled = true;
            }
            break;
         case GLFW_KEY_UP:
            {
                sequencer->editor->changeCursorPitch(1);
                handled = true;
            }
            break;
        case GLFW_KEY_DOWN:
            {
                sequencer->editor->changeCursorPitch(-1);
                handled = true;
            }
            break;

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
                if (ctrl) {
                    sequencer->editor->insertPresetNote(MidiEditor::Durations::Eighth);
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_H:
            {
                if (ctrl) {
                    sequencer->editor->insertPresetNote(MidiEditor::Durations::Half);
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_P:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Pitch);
                handled = true;
            }
            break;
        case GLFW_KEY_Q:
            {
                if (ctrl) {
                    sequencer->editor->insertPresetNote(MidiEditor::Durations::Quarter);
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_S:
            {
                if (!ctrl) {
                    sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::StartTime);
                } else {
                    sequencer->editor->insertPresetNote(MidiEditor::Durations::Sixteenth);
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
                if (ctrl) {
                    sequencer->editor->insertPresetNote(MidiEditor::Durations::Whole);
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_X:
            {
                if (ctrl) {
                    sequencer->editor->cut();
                    handled = true;
                }
            }
            break;
        case GLFW_KEY_KP_0:
        case GLFW_KEY_INSERT:
            sequencer->editor->insertNote();
            handled = true;
            break;
        case GLFW_KEY_BACKSPACE:
        case GLFW_KEY_KP_DECIMAL:
        case GLFW_KEY_DELETE:
            sequencer->editor->deleteNote();
            handled = true;
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