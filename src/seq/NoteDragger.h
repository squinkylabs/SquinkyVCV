#pragma once

class NoteDisplay;

#include "SqMath.h"
struct NVGcontext;

class NoteDragger
{
public:
    NoteDragger(NoteDisplay*, const sq::Vec& pos);
    virtual ~NoteDragger();
    virtual void onDrag(const sq::Vec& pos)=0;
    virtual void commit()=0;
    virtual void draw(NVGcontext *vg)=0;
   
protected:
    NoteDisplay* const host;
    const sq::Vec startPos;
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(NoteDisplay*,const sq::Vec& pos);
private:
   
    void onDrag(const sq::Vec& pos) override;
    void commit() override;
    void draw(NVGcontext *vg) override;
};
