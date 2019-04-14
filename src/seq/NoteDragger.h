#pragma once

class NoteDisplay;

class NoteDragger
{
public:
    NoteDragger(NoteDisplay*);
    virtual ~NoteDragger();
    virtual void onDrag()=0;
    virtual void commit()=0;
   
protected:
    NoteDisplay* const host;
};

class NotePitchDragger : public NoteDragger
{
public:
    NotePitchDragger(NoteDisplay*);
    void onDrag() override;
    void commit() override;
};
