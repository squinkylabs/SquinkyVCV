#pragma once

class NoteDisplay;

#include "SqMath.h"

struct NVGcontext;
class NoteDisplay;
class MidiNoteEvent;
class MidiSequencer;
class MidiSequencer;
//using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;


/**
 * Base class for all drag operations
 *
 * Currently, the way it works is that as you drag,
 * the only thing that changes is the mouse pos.
 *
 * Also, some overridden onDrag handler know how to scroll the viewport
 * allowind dragging to locations that would be off the screen.
 *
 * When the mouse is released, "commit" is called. That
 * does the persistent edits.
 */

class NoteDragger
{
public:
    /**
     * x, y is the initial mouse position in screen coordinates
     */
    NoteDragger(MidiSequencerPtr, float x, float y);
    virtual ~NoteDragger();
    virtual void onDrag(float deltaX, float deltaY);
    virtual void commit() = 0;
    virtual void draw(NVGcontext *vg) = 0;

    virtual bool willDrawSelection() const
    {
        return true;        // for now, they all do
    }

    /**
     * tweaks an a time shift into one that will follow the grid.
     * time units are all pixel shifts
     */
    virtual float quantizeForDisplay(float metricTime, float timeShiftPixels, bool canGoBelowGridSize);
protected:
    MidiSequencerPtr sequencer;
    const float startX;
    const float startY;
    float curMousePositionX = 0;
    float curMousePositionY = 0;

    /**
     * shifts are in units of one pixel
     */
    void drawNotes(NVGcontext *vg, float verticalShift, float horizontalShift, float horizontalStretch);
};

/**
 * Concrete implentation for dragging notes up and down in pitch
 */
class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(MidiSequencerPtr, float x, float y);
    void onDrag(float deltaX, float deltaY) override;

private:

    void commit() override;
    void draw(NVGcontext *vg) override;

// TODO: do we use any of the below???
    /**
     * Calculate the transpose based on how far the mouse has moved.
     * Units are pitch units.
     */
    float calcTranspose() const;

    /**
     * Calculate how much the viewport must be shifted to
     * keep the transposed notes in range.
     */
    float calcShift(float transpose) const;

    const float viewportUpperPitch0;    // The initial pitch of the topmost pixel in the viewport
    const float highPitchForDragStart;  // The pitch at which we start dragging up
    const float viewportLowerPitch0;    // The initial pitch of the bottom most pixel in the viewport
    const float lowPitchForDragStart;   // The pitch at which we start dragging down

    const float pitch0;                     // pitch when the drag started
};


/**
 * Base class for draggers the drag left and right.
 */
class NoteHorizontalDragger : public NoteDragger
{
public:
    NoteHorizontalDragger(MidiSequencerPtr, float x, float y, float initialNoteValue);
    void onDrag(float deltaX, float deltaY) override;
    float quantizeForDisplay(float metricTime, float shiftInMetricTime, bool canGoBelowGridSize) override;
protected:

    /**
     * Calculate the time shift based on how far the mouse has moved.
     * Units are midi time.
     */
    float calcTimeShift() const;

    /**
     * Calculate how much the viewport must be shifted to
     * keep the shifted notes in range.
     */
    float calcViewportShift(float transpose) const;

    const float viewportStartTime0;     // The initial time of the leftmost pixel in the viewport
    const float viewportEndTime0;       // The initial time of the rightmost pixel in the viewport

    const float time0;                  // time (on screen) when the drag started

    /**
     * Attribute of the note clicked on to start the drag.  
     * start time of first note or duration.
     * 
     * TODO: are we using this?
     */
    const float initialNoteValue;           

};

/**
 * concrete implementation for dragging the start times of notes
 */
class NoteStartDragger : public NoteHorizontalDragger
{
public:
    NoteStartDragger(MidiSequencerPtr, float x, float y, float initialStartTime);
    void commit() override;
    void draw(NVGcontext *vg) override;
};

class NoteDurationDragger : public NoteHorizontalDragger
{
public:
    NoteDurationDragger(MidiSequencerPtr, float x, float y, float initialDuration);
    void commit() override;
    void draw(NVGcontext *vg) override;
};