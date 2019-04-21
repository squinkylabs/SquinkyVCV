#pragma once

#include <memory>

class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

class MouseManager
{
public:
    MouseManager(MidiSequencerPtr);

    /**
     * Handler for primary mouse button
     * 
     * isPressed is true on a button press, false on button release.
     * ctrl is true if control key is down.
     * shift is true if shift key is down.
     */
    bool onMouseButton(float x, float y, bool isPressed, bool ctrl, bool shift);
private:
    MidiSequencerPtr sequencer;
    
};