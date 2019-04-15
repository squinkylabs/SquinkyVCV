#pragma once

class NoteDisplay;

#include "SqMath.h"

class NoteDragger
{
public:
    NoteDragger(NoteDisplay*, const sq::Vec& pos);
    virtual ~NoteDragger();
    virtual void onDrag()=0;
    virtual void commit()=0;
   
protected:
    NoteDisplay* const host;
    const sq::Vec startPos;
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(NoteDisplay*,const sq::Vec& pos);
    void onDrag() override;
    void commit() override;
};
