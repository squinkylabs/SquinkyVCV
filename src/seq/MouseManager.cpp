
#include "MidiKeyboardHandler.h"    // TODO: get rid of this
#include "MidiSequencer.h"
#include "MouseManager.h"
#include "NoteScreenScale.h"

MouseManager::MouseManager(MidiSequencerPtr seq) : sequencer(seq)
{

}

bool MouseManager::onMouseButton(float x, float y, bool isPressed, bool ctrl, bool shift)
{
    printf("mouse manager click\n");
    bool ret = false;
    // old handler logic
    auto scaler = sequencer->context->getScaler();
    assert(scaler);
    bool bInBounds = scaler->isPointInBounds(x, y);
    if (bInBounds) {
        const float time = scaler->xToMidiTime(x);
        const float pitchCV = scaler->yToMidiCVPitch(y);
        MidiKeyboardHandler::doMouseClick(sequencer, time, pitchCV, shift, ctrl);
        ret = true;
    }
    return ret;
}
