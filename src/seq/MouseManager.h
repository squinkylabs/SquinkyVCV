#pragma once

#include <memory>

class MidiSequencer;
struct NVGcontext;
class NoteDisplay;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

/**
 * This class only exists to isolate the mouse
 * handler from NoteDisplay, so that we can do
 * conditional builds.
 */
class MouseManager
{
public:
    MouseManager(MidiSequencerPtr);

    void draw(NVGcontext *vg);
    /**
     * Handler for primary mouse button
     *
     * isPressed is true on a button press, false on button release.
     * ctrl is true if control key is down.
     * shift is true if shift key is down.
     */
    bool onMouseButton(float x, float y, bool isPressed, bool ctrl, bool shift);

    /**
     * drag handlers
     */
    bool onDragStart();
    bool onDragEnd();
    bool onDragMove(float x, float y);
    bool willDrawSelection() const;

    bool onDoubleClick();
private:

    MidiSequencerPtr sequencer;
    float lastMouseClickPosX = 0;
    float lastMouseClickPosY = 0;
    std::shared_ptr<class NoteDragger> noteDragger;
    bool mouseClickWasIgnored = false;
    bool mouseMovedWhileDragging = false;

    std::tuple<bool, float, float> xyToTimePitch(float x, float y) const;

    static void doMouseClick( MidiSequencerPtr sequencer, float time, float pitchCV,
        bool shiftKey, bool ctrlKey);

};