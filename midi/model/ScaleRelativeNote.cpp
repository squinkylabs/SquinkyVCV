#include "ScaleRelativeNote.h"

ScaleRelativeNote::ScaleRelativeNote(int degree, int octave, ScalePtr scale) :
    degree(degree),
    octave(octave),
    scale(scale),
    valid(true)
{
    
}
