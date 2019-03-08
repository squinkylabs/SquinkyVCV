
#include "MidiKeyboardHandler.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>

#include <assert.h>

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
      //  case GLFW_KEY_PLUS:
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_LEFT:
        case GLFW_KEY_UP:
        case GLFW_KEY_DOWN:
            doIt = true;
    }
    return doIt;
}

void MidiKeyboardHandler::handleNoteEditorChange(
    MidiSequencer* sequencer,
    ChangeType type,
    bool increase)
{
    assert(type != ChangeType::lessThan); // can't handle
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
                int units = (type == ChangeType::bracket) ? 4 : 1;
                if (!increase) {
                    units = -units;
                }
                sequencer->editor->changeDuration(false, units);
            }
            break;

        case MidiEditorContext::NoteAttribute::StartTime:
            {
                int units = (type == ChangeType::bracket) ? 4 : 1;
                if (!increase) {
                    units = -units;
                }
                sequencer->editor->changeStartTime(false, units);
            }
            break;
    }
}

extern void sequencerHelp();

bool MidiKeyboardHandler::handle(
    MidiSequencer* sequencer,
    unsigned key,
    unsigned mods)
{
    bool handled = false;
    const bool shift = (mods & GLFW_MOD_SHIFT);
    const bool ctrl = (mods & GLFW_MOD_CONTROL);
   
    switch(key) {
        case GLFW_KEY_F1:
            sequencerHelp();
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
        case GLFW_KEY_P:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Pitch);
                handled = true;
            }
            break;
        case GLFW_KEY_D:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Duration);
                 handled = true;
            }
            break;
        case GLFW_KEY_S:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::StartTime);
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
        case GLFW_KEY_KP_DECIMAL:
        case GLFW_KEY_DELETE:
            sequencer->editor->deleteNote();
            handled = true;
            break;
        case GLFW_KEY_Z:
            if (ctrl & !shift) {
                handled = true;
                if (sequencer->undo->canUndo()) {
                    sequencer->undo->undo();
                } 
            } else if (ctrl & shift) {
                if (sequencer->undo->canRedo()) {
                    sequencer->undo->redo();  
                }
            }
            break;
     
         case GLFW_KEY_Y:
            if (ctrl) {
                handled = true;
                if (sequencer->undo->canRedo()) {
                    sequencer->undo->redo();
                } 
            }
            break;
    }
    return handled;
}
#endif