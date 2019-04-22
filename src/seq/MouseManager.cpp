
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


std::tuple<bool, float, float> MouseManager::xyToTimePitch(float x, float y) const
{
    auto scaler = sequencer->context->getScaler();
    assert(scaler);
    bool bInBounds = scaler->isPointInBounds(x, y);
    float time=0, pitchCV=0;
    if (bInBounds) {
        time = scaler->xToMidiTime(x);
        pitchCV = scaler->yToMidiCVPitch(y);
       // MidiKeyboardHandler::doMouseClick(sequencer, time, pitchCV, shift, ctrl);
       // ret = true;
    }
    return std::make_tuple(bInBounds, time, pitchCV);
}

bool MouseManager::onMouseButton(float x, float y, bool isPressed, bool ctrl, bool shift)
{
    bool ret = false;

    lastMouseClickPosX = x;
    lastMouseClickPosY = y;

    auto timeAndPitch = xyToTimePitch(x, y);
    if (!std::get<0>(timeAndPitch)) {
        // if the mouse click is not in bounds, ignore it
        return false;
    }

    const float time = std::get<1>(timeAndPitch);
    const float pitchCV = std::get<2>(timeAndPitch);

    // This will move the cursor, which we may not want all the time
    MidiNoteEventPtr curNote = sequencer->editor->moveToTimeAndPitch(time, pitchCV);
    bool curNoteIsSelected = sequencer->selection->isSelected(curNote);


    if ((isPressed && curNote && !curNoteIsSelected) || 
            (!isPressed && mouseClickWasIgnored)) {
        printf("onMouseButton calling doMouseClick\n"); fflush(stdout);
        mouseClickWasIgnored = false;

        // TODO: use more specific handler calls, get rid of this ambiguous catchall
        MidiKeyboardHandler::doMouseClick(
            sequencer,
            std::get<1>(timeAndPitch),
            std::get<2>(timeAndPitch),
            shift,
            ctrl);
        ret = true;
    } else {
        mouseClickWasIgnored = true;
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
