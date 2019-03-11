#pragma once

class MidiEditorContext;



#include "MidiEvent.h"
/**
 * This class know how to map between pitch, time, and screen coordinates.
 * notes on the screen have:
 *      height in pixels - determined by vertical zoom
 *      width in pixels - determined by duration and horizontal zoom
 *      x position where the note starts.
 *      y position of the upper edge of the notes.
 *
 * Coordinate conventions:
 *      if viewport hi and low pitches are the same, it maps a note of that pitch to full screen.
 *      y==0 it the top edge, increasing y goes down the screen
 */

class NoteScreenScale
{
public:
    NoteScreenScale(
        float screenWidth,
        float screenHeight,
        float hMargin,        // units of empty space l and r (won't be pixels if zoom != 1).
        float topMargin
    );
    void setContext(std::shared_ptr<MidiEditorContext>);

    /**
     * update internal match to reflect new state of edit context
     */
    void reCalculate();


    float midiTimeToX(const MidiEvent& ev);
    float midiTimeToX(MidiEvent::time_t ev);
    float midiTimeTodX(MidiEvent::time_t dt);

    std::pair<float, float> midiTimeToHBounds(const MidiNoteEvent& note);



    float midiPitchToY(const MidiNoteEvent& note);
    float midiCvToY(float cv);

    float noteHeight();

    void assertValid() const;
private:
    float unitsPerPix = 1;
    float by = 0;
    float bx = 0;
    float ax = 0;
    float ay = 0;
    std::weak_ptr<MidiEditorContext> _context;
    std::shared_ptr<MidiEditorContext> context() const;

    const float screenWidth;
    const float screenHeight;
    const float hMargin;
    const float topMargin;
};