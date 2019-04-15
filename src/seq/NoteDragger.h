#pragma once

class NoteDisplay;

#include "math.hpp"

using Vec = rack::math::Vec;

class NoteDragger
{
public:
    NoteDragger(NoteDisplay*, const Vec& pos);
    virtual ~NoteDragger();
    virtual void onDrag()=0;
    virtual void commit()=0;
   
protected:
    NoteDisplay* const host;
    const Vec startPos;
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(NoteDisplay*,const Vec& pos);
    void onDrag() override;
    void commit() override;
};
