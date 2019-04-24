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

   
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(MidiSequencerPtr, float x, float y);

private:
   
    void commit() override;
    void draw(NVGcontext *vg) override;
    void drawNotes(NVGcontext *vg);
};
