
#include "MidiKeyboardHandler.h"    // TODO: get rid of this
#include "MidiSequencer.h"
#include "MouseManager.h"
#include "NoteDragger.h"
#include "NoteScreenScale.h"

#ifdef _SEQ
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

    if (!isPressed && noteDragger && mouseMovedWhileDragging) {
        // Mouse up without any dragging get processed.
        // Mouse up with drag does not -> dragger will finish
        return false;
    }

    const float time = std::get<1>(timeAndPitch);
    const float pitchCV = std::get<2>(timeAndPitch);

    // This will move the cursor, which we may not want all the time
    MidiNoteEventPtr curNote = sequencer->editor->moveToTimeAndPitch(time, pitchCV);
    bool curNoteIsSelected = sequencer->selection->isSelected(curNote);


    if ((isPressed && curNote && !curNoteIsSelected) || 
            (!isPressed && mouseClickWasIgnored)) {
        // printf("onMouseButton calling doMouseClick\n"); fflush(stdout);
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
   // printf("MouseManger::onDragStart()\n"); fflush(stdout);
    mouseMovedWhileDragging = false;
    MidiNoteEventPtr note = sequencer->editor->getNoteUnderCursor();
    if (!note) {
        return true;
    }
    auto scaler = sequencer->context->getScaler();
    const float start = note->startTime;
    const float end = note->endTime();
    const float cursorTime =sequencer->context->cursorTime();


    const float relativeTime = (cursorTime - start) / (end - start);
    //printf("relative time = %f\n", relativeTime); fflush(stdout);

    if (relativeTime <= .33f) {
        noteDragger = std::make_shared<NoteStartDragger>(sequencer, lastMouseClickPosX, lastMouseClickPosY);     
    } else if (relativeTime <= .66f) {
        noteDragger = std::make_shared<NotePitchDragger>(sequencer, lastMouseClickPosX, lastMouseClickPosY); 
    } else {
        noteDragger = std::make_shared<NoteDurationDragger>(sequencer, lastMouseClickPosX, lastMouseClickPosY); 
    }
    return true;
}

bool MouseManager::onDragEnd()
{
  //   printf("MouseManger::onDragEnd()\n"); fflush(stdout);
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
    if (x!=0 || y != 0) {
        mouseMovedWhileDragging = true;
    }
    return ret;
}

bool MouseManager::willDrawSelection() const
{
    bool ret = false;
    if (noteDragger) {
        ret = noteDragger->willDrawSelection();
    }
    return ret;
}
#endif
