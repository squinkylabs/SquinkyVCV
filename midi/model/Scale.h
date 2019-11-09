#pragma once

#include <memory>

class Scale;
class ScaleRelativeNote;
using ScalePtr = std::shared_ptr<Scale>;
using ScaleRelativeNotePtr = std::shared_ptr<ScaleRelativeNote>;

class Scale
{
public:
    enum class Scales {
        Major,
        Dorian,
        Phrygian,
        Lydian,
        Mixolydian,
        Minor,
        Locrian
    };

    static ScalePtr getScale(Scales, int);

    ScaleRelativeNotePtr getScaleRelativeNote(int semitone);
private:
    Scale()
    {

    }
};