
#include "MidiKeyboardHandler.h"    // TODO: get rid of this
#include "MidiSequencer.h"
#include "MouseManager.h"
#include "NoteDragger.h"
#include "NoteScreenScale.h"

MouseManager::MouseManager(MidiSequencerPtr seq) 
    : sequencer(seq)
{

}

void MouseManager::draw(NVGcontext *vg)
{
    if (noteDragger) {
        noteDragger->draw(vg);
    }
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

bool MouseManager::onDragStart()
{
    printf("MouseManger::onDragStart()\n"); fflush(stdout);
    noteDragger = std::make_shared<NotePitchDragger>(lastMouseClickPosX, lastMouseClickPosY); 
    return true;
}

bool MouseManager::onDragEnd()
{
     printf("MouseManger::onDragEnd()\n"); fflush(stdout);
    bool ret = false;
    if (noteDragger) {
        noteDragger->commit();
        noteDragger.reset();
        ret = true;
    }
    return ret;
}

bool MouseManager::onDragMove(float x, float y)
{
    bool ret = false;
    if (noteDragger) {
        noteDragger->onDrag(x, y);
        ret=true;
    }
    return ret;
}
