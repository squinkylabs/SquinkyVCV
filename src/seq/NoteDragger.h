#pragma once

class NoteDisplay;

#include "SqMath.h"
struct NVGcontext;
class NoteDisplay;

class NoteDragger
{
public:
    /**
     * x, y is the initial mouse position in screen coordinates
     */
    NoteDragger(float x, float y);
    virtual ~NoteDragger();
    virtual void onDrag(float deltaX, float deltaY);
    virtual void commit()=0;
    virtual void draw(NVGcontext *vg)=0;
   
protected:
   // NoteDisplay* const host;
   // const sq::Vec startPos;
    const float startX;
    const float startY;
   // sq::Vec curMousePosition;
   float curMousePositionX = 0;
   float curMousePositionY = 0;
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(float x, float y);

private:
   
    void commit() override;
    void draw(NVGcontext *vg) override;
};
