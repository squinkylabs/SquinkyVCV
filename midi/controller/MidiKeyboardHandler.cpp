
#include "MidiKeyboardHandler.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>

bool MidiKeyboardHandler::handle(MidiSequencer* sequencer,
    unsigned key, unsigned mods)
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
            sequencer->editor->transpose( 1 / 12.f);
            handled = true;
            break;
        case GLFW_KEY_EQUAL:
            if (shift) {
                sequencer->editor->transpose( 1 / 12.f);
                handled = true;
            }
            break;
        case GLFW_KEY_KP_SUBTRACT:
            sequencer->editor->transpose( -1 / 12.f);
            handled = true;
            break;
        case GLFW_KEY_MINUS:
            if (!shift) {
                sequencer->editor->transpose( -1 / 12.f);
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
    }
    return handled;
}