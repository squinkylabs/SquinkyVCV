#pragma once
#include <memory>

class Scale;
class ScaleRelativeNote;
using ScalePtr = std::shared_ptr<Scale>;
using ScaleRelativeNotePtr = std::shared_ptr<ScaleRelativeNote>;

class ScaleRelativeNote
{
public:
    ScaleRelativeNote(int degree, int octave, ScalePtr scale);
    // This ctor makes an invalid one
    ScaleRelativeNote() : valid(false), degree(-1), octave(0)
    {

    }

    bool isSameDegree(const ScaleRelativeNote& other);

    bool isValid() const;
private:
    const bool valid; 
    const int degree;
    const int octave;
    ScalePtr scale;

};