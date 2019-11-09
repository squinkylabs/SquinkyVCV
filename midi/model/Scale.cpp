#include "Scale.h"
#include "ScaleRelativeNote.h"

ScalePtr Scale::getScale(Scale::Scales scale, int root)
{
    Scale* p = new Scale();
    return ScalePtr(p);
}

ScaleRelativeNotePtr Scale::getScaleRelativeNote(int semitone)
{
    return ScaleRelativeNotePtr(new ScaleRelativeNote());
}