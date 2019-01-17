
#include "MidiKeyboardHandler.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>

bool MidiKeyboardHandler::handle(MidiSequencer* sequencer,
    unsigned key, unsigned mods)
{
    bool handled = false;
    const bool shift = (mods & GLFW_MOD_SHIFT);
   
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
    }
    return handled;
}