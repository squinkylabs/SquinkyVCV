#pragma once

class NoteDisplay;

#include "SqMath.h"
struct NVGcontext;

class NoteDragger
{
public:
    /**
     * pos is the initial mouse position in screen coordinates
     */
    NoteDragger(NoteDisplay*, const sq::Vec& pos);
    virtual ~NoteDragger();
    virtual void onDrag(const sq::Vec& pos);
    virtual void commit()=0;
    virtual void draw(NVGcontext *vg)=0;
   
protected:
    NoteDisplay* const host;
    const sq::Vec startPos;
    sq::Vec curMousePosition;
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(NoteDisplay*,const sq::Vec& pos);

private:
   
    void commit() override;
    void draw(NVGcontext *vg) override;
};
