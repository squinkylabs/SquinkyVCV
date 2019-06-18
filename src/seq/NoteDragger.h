#pragma once

class NoteDisplay;

#include "SqMath.h"

struct NVGcontext;
class NoteDisplay;
class MidiSequencer;
class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;


/**
 * Base class for all drag operations
 * 
 * currently, the way it works is that as you drag, 
 * the only thing that changes is the mouse pos.
 * 
 * When we draw the overridden draw function draw the 
 * displaced notes as if they are "dragging".
 * 
 * When the mouse is released, "commit" is called. That
 * does the persistent edits.
 * --------------------------------------
 * 
 * We would like for dragging off the canvas to scroll the
 * viewport. that should be relatively easy, and could be
 * implemented in the base class. To do this:
 * 
 *      drawing will have to get smarter. now it's just delta = movement of mousem
 *      but it will need to subtract changes in viewport.
 * 
 *      dragging will introduce undesired undo events.
 * 
 *      
 * so, for drawing, drawDelta = mouseDelta - viewportDelta.
 * remember that  auto scaler = sequencer->context->getScaler();
 * the scaler relates screen coordinated to music ones.
 * will need helpers to know how far out of bounds the cursor is.
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
    virtual void commit()=0;
    virtual void draw(NVGcontext *vg)=0;
   
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

    /**
     * returns 0 if cursor is still inviewport
     *          amount outside, if not (+/-), in pitch CV
     */
    float getCursorOutsidePitchRange() const;
    
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(MidiSequencerPtr, float x, float y);
    void onDrag(float deltaX, float deltaY) override;

private:
   
    void commit() override;
    void draw(NVGcontext *vg) override;
};

class NoteStartDragger : public NoteDragger
{
public:
    NoteStartDragger(MidiSequencerPtr, float x, float y);
    void commit() override;
    virtual void draw(NVGcontext *vg) override;
};

class NoteDurationDragger : public NoteDragger
{
public:
    NoteDurationDragger(MidiSequencerPtr, float x, float y);
    void commit() override;
    virtual void draw(NVGcontext *vg) override;
};