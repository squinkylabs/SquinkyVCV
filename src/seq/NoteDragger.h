#pragma once

class NoteDisplay;

#include "SqMath.h"

struct NVGcontext;
class NoteDisplay;
class MidiSequencer;
class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

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

   
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(MidiSequencerPtr, float x, float y);

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