
#include "MidiKeyboardHandler.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>

#include <assert.h>

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

bool MidiKeyboardHandler::handle(
    MidiSequencer* sequencer,
    unsigned key,
    unsigned mods)
{
    bool handled = false;
    const bool shift = (mods & GLFW_MOD_SHIFT);
    const bool ctrl = (mods & GLFW_MOD_CONTROL);
   
    switch(key) {
        case GLFW_KEY_TAB: 
            if (shift) {
                sequencer->editor->selectPrevNote();
            } else {
                sequencer->editor->selectNextNote();
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
        case GLFW_KEY_P:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Pitch);
            }
            break;
        case GLFW_KEY_D:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::Duration);
            }
            break;
        case GLFW_KEY_S:
            {
                sequencer->editor->setNoteEditorAttribute(MidiEditorContext::NoteAttribute::StartTime);
            }
            break;
    }
    return handled;
}