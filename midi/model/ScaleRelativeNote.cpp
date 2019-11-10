#include "ScaleRelativeNote.h"

ScaleRelativeNote::ScaleRelativeNote(int degree, int octave, ScalePtr scale) :
    degree(degree),
    octave(octave),
    scale(scale),
    valid(true)
{
    
}

#if 0
bool ScaleRelativeNote::isValid() const
{
    return valid;
}

bool ScaleRelativeNote::isSameDegree(const ScaleRelativeNote& other)
{
    return degree == other.degree;
}
#endif