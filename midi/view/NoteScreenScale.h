#pragma once

class MidiViewport;



#include "MidiEvent.h"
/**
 * This class know how to map between pitch, time, and screen coordinates
 */
class NoteScreenScale
{
public:
    NoteScreenScale(MidiViewport& vp, float screenWidth, float screenHeight);
    float midiTimeToX(const MidiEvent& ev);
    float midiTimeTodX(MidiEvent::time_t dt);
    float midiPitchToY(const MidiNoteEvent& note);
private:
    float ax = 0;
    float ay = 0;
    MidiViewport& viewport;
};