#pragma once

class MidiEditorContext;

#include "MidiEvent.h"

/**
 * This class knows how to map between pitch, time, and screen coordinates.
 * Notes on the screen have:
 *      height in pixels - determined by vertical zoom
 *      width in pixels - determined by duration and horizontal zoom
 *      x position where the note starts.
 *      y position of the upper edge of the notes.
 *
 * Coordinate conventions:
 *      if viewport hi and low pitches are the same, it maps a note of that pitch to full screen.
 *      y==0 it the top edge, increasing y goes down the screen (lower pitch)
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
    void assertValid() const;

    /**
     * Convert time to x position
     */
    float midiTimeToX(const MidiEvent& ev) const;
    float midiTimeToX(MidiEvent::time_t ev) const;
    float midiTimeTodX(MidiEvent::time_t dt) const;
    std::pair<float, float> midiTimeToHBounds(const MidiNoteEvent& note) const;

    /** Convert pitch to Y position
     */
    float midiPitchToY(const MidiNoteEvent& note) const;
    float midiCvToY(float cv) const;
    float noteHeight() const;

    /** Convert x position to time
     */
    float xToMidiTime(float) const;
    float xToMidiDeltaTime(float);

    /** Convert y position to pitch
     *  Will quantize the pitch to the nearest semitone.
     */
    float yToMidiCVPitch(float) const;

    /** convert y axis range to change in pitch
     */
    float yToMidiDeltaCVPitch(float) const;

    bool isPointInBounds(float x, float y) const;
private:
    /** These are the linear equation coefficients
     * for mapping from music time/pitch to screen
     */
    float by = 0;
    float bx = 0;
    float ax = 0;
    float ay = 0;

     /** These are the linear equation coefficients
     * for mapping from screen coordinates to music time/pitch
     */
    float by_rev = 0;
    float bx_rev = 0;
    float ax_rev = 0;
    float ay_rev = 0;


    float unitsPerPix = 1;
    std::weak_ptr<MidiEditorContext> _context;
    std::shared_ptr<MidiEditorContext> context() const;

    const float screenWidth;
    const float screenHeight;
    const float hMargin;
    const float topMargin;
};