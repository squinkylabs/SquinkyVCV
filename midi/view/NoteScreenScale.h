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

    std::pair<float, float> midiTimeToHBounds(const MidiNoteEvent& note);


    /** notes on the screen have:
     *      height in pixels - determined by vertial zoom
     *      width in pixels - determined by duration and horizontal zoom
     *      x position where the note starts.
     *      y position of the upper edge of the notes
     */
     float midiPitchToY(const MidiNoteEvent& note);
private:
    float ax = 0;
    float ay = 0;
    MidiViewport& viewport;
};